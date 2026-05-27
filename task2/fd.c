#include "FAT.h"
int fs_open(int pid, const char *filename, int mode)
{
    if (pid < 0 || pid >= MAX_PROCESS || filename == NULL) {
        return -1;
    }

    int dir_index = -1;

    for (int i = 0; i < MAX_ENTRY; i++) {
        if (Directory[i].ac == 1 &&
            Directory[i].parent == current_dir &&
            Directory[i].type == 0 &&
            strcmp(Directory[i].name, filename) == 0) {
            dir_index = i;
            break;
        }
    }

    if (dir_index == -1) {
        printf("open failed: file not found: %s\n", filename);
        return -1;
    }

    pthread_mutex_lock(&fs_mutex);

    if (mode == MODE_READ) {
        if (Directory[dir_index].writer_pid != -1) {
            pthread_mutex_unlock(&fs_mutex);
            printf("open failed: file is being written\n");
            return -1;
        }

        Directory[dir_index].reader_count++;
    } 
    else if (mode == MODE_WRITE || mode == MODE_RDWR) {
        if (Directory[dir_index].writer_pid != -1 ||
            Directory[dir_index].reader_count > 0) {
            pthread_mutex_unlock(&fs_mutex);
            printf("open failed: file is busy\n");
            return -1;
        }

        Directory[dir_index].writer_pid = pid;
    } 
    else {
        pthread_mutex_unlock(&fs_mutex);
        printf("open failed: invalid mode\n");
        return -1;
    }

    pthread_mutex_unlock(&fs_mutex);

    for (int fd = 0; fd < MAX_FD; fd++) {
        if (ProcessTable[pid].fd_table[fd].used == 0) {
            ProcessTable[pid].fd_table[fd].used = 1;
            ProcessTable[pid].fd_table[fd].dir_index = dir_index;
            ProcessTable[pid].fd_table[fd].offset = 0;
            ProcessTable[pid].fd_table[fd].mode = mode;

            printf("open success: pid=%d, file=%s, fd=%d\n",
                   pid, filename, fd);
            return fd;
        }
    }

    // fd 表满了，必须回滚刚才加的读写锁
    pthread_mutex_lock(&fs_mutex);

    if (mode == MODE_READ) {
        if (Directory[dir_index].reader_count > 0) {
            Directory[dir_index].reader_count--;
        }
    } else if (mode == MODE_WRITE || mode == MODE_RDWR) {
        if (Directory[dir_index].writer_pid == pid) {
            Directory[dir_index].writer_pid = -1;
        }
    }

    pthread_mutex_unlock(&fs_mutex);

    printf("open failed: fd table full\n");
    return -1;
}

bool fs_close(int pid, int fd)
{
    if (pid < 0 || pid >= MAX_PROCESS || fd < 0 || fd >= MAX_FD) {
        return false;
    }

    if (ProcessTable[pid].fd_table[fd].used == 0) {
        printf("close failed: invalid fd\n");
        return false;
    }

    int dir_index = ProcessTable[pid].fd_table[fd].dir_index;
    int mode = ProcessTable[pid].fd_table[fd].mode;

    pthread_mutex_lock(&fs_mutex);

    if (mode == MODE_READ) {
        if (Directory[dir_index].reader_count > 0) {
            Directory[dir_index].reader_count--;
        }
    } 
    else if (mode == MODE_WRITE || mode == MODE_RDWR) {
        if (Directory[dir_index].writer_pid == pid) {
            Directory[dir_index].writer_pid = -1;
        }
    }

    pthread_mutex_unlock(&fs_mutex);

    ProcessTable[pid].fd_table[fd].used = 0;
    ProcessTable[pid].fd_table[fd].dir_index = -1;
    ProcessTable[pid].fd_table[fd].offset = 0;
    ProcessTable[pid].fd_table[fd].mode = 0;

    printf("close success: pid=%d, fd=%d\n", pid, fd);
    return true;
}