#include "FAT.h"
//创建文件
bool fs_create(const char *filename) {
    for (int i = 0; i < MAX_ENTRY; i++) {
        if (Directory[i].ac == 0) { //找到空闲目录项
            strncpy(Directory[i].name, filename, 16);
            Directory[i].size = 0;
            Directory[i].start_block = -1;  
            Directory[i].ac = 1; //标记为已使用
            Directory[i].type = 0; //0表示文件
            Directory[i].parent = current_dir; //设置父目录索引
            Directory[i].reader_count = 0; // 初始化读者数量  
            Directory[i].writer_pid = -1; // 初始化写者PID
            printf("File created successfully: %s\n", filename);
            return true;
        }
    }
    printf("Failed to create file: %s\n", filename);
    return false; //文件创建失败
}

//写文件
int fs_write(int pid, int fd, const char *data, int size)
{
    if (pid < 0 || pid >= MAX_PROCESS ||
        fd < 0 || fd >= MAX_FD ||
        data == NULL || size < 0) {
        return -1;
    }

    FileDescriptor *desc = &ProcessTable[pid].fd_table[fd];

    if (desc->used == 0) {
        printf("write failed: fd is not open\n");
        return -1;
    }

    if (desc->mode != MODE_WRITE && desc->mode != MODE_RDWR) {
        printf("write failed: fd is not writable\n");
        return -1;
    }

    if (size == 0) {
        return 0;
    }
    pthread_mutex_lock(&fs_mutex);

    int dir_index = desc->dir_index;
    int offset = desc->offset;
    int end_pos = offset + size;
    int need_blocks = (end_pos + BLOCK_SIZE - 1) / BLOCK_SIZE;

    if (Directory[dir_index].start_block == -1) {
        int new_block = -1;

        for (int i = 0; i < BLOCK_NUM; i++) {
            if (FAT[i] == 0) {
                new_block = i;
                break;
            }
        }

        if (new_block == -1) {
            printf("write failed: no free block\n");
            return -1;
        }

        FAT[new_block] = -1;
        memset(Disk + new_block * BLOCK_SIZE, 0, BLOCK_SIZE);
        Directory[dir_index].start_block = new_block;
    }

    int cur = Directory[dir_index].start_block;
    int block_count = 1;

    while (FAT[cur] != -1) {
        cur = FAT[cur];
        block_count++;
    }

    while (block_count < need_blocks) {
        int new_block = -1;

        for (int i = 0; i < BLOCK_NUM; i++) {
            if (FAT[i] == 0) {
                new_block = i;
                break;
            }
        }

        if (new_block == -1) {
            printf("write failed: no enough space\n");
            return -1;
        }

        FAT[new_block] = -1;
        memset(Disk + new_block * BLOCK_SIZE, 0, BLOCK_SIZE);

        FAT[cur] = new_block;
        cur = new_block;
        block_count++;
    }

    int block_index = offset / BLOCK_SIZE;
    int block_offset = offset % BLOCK_SIZE;

    cur = Directory[dir_index].start_block;

    for (int i = 0; i < block_index; i++) {
        cur = FAT[cur];
    }

    int bytes_written = 0;

    while (cur != -1 && bytes_written < size) {
        int write_size = BLOCK_SIZE - block_offset;

        if (write_size > size - bytes_written) {
            write_size = size - bytes_written;
        }

        memcpy(
            Disk + cur * BLOCK_SIZE + block_offset,
            data + bytes_written,
            write_size
        );

        bytes_written += write_size;
        block_offset = 0;

        if (bytes_written < size) {
            cur = FAT[cur];
        }
    }

    desc->offset += bytes_written;

    if (desc->offset > Directory[dir_index].size) {
        Directory[dir_index].size = desc->offset;
    }
    pthread_mutex_unlock(&fs_mutex);    
    return bytes_written;
}
//读文件
int fs_read(int pid, int fd, char *buffer, int size)
{
    if (pid < 0 || pid >= MAX_PROCESS ||
        fd < 0 || fd >= MAX_FD ||
        buffer == NULL || size < 0) {
        return -1;
    }

    FileDescriptor *desc = &ProcessTable[pid].fd_table[fd];

    if (desc->used == 0) {
        printf("read failed: fd is not open\n");
        return -1;
    }

    if (desc->mode != MODE_READ && desc->mode != MODE_RDWR) {
        printf("read failed: fd is not readable\n");
        return -1;
    }

    pthread_mutex_lock(&fs_mutex);

    int dir_index = desc->dir_index;
    int file_size = Directory[dir_index].size;
    int offset = desc->offset;

    if (offset >= file_size) {
        buffer[0] = '\0';
        pthread_mutex_unlock(&fs_mutex);
        return 0;
    }

    int bytes_to_read = size;

    if (offset + bytes_to_read > file_size) {
        bytes_to_read = file_size - offset;
    }

    int bytes_read = 0;
    int cur = Directory[dir_index].start_block;
    int skip_blocks = offset / BLOCK_SIZE;
    int block_offset = offset % BLOCK_SIZE;

    for (int i = 0; i < skip_blocks && cur != -1; i++) {
        cur = FAT[cur];
    }

    while (cur != -1 && bytes_read < bytes_to_read) {
        int read_size = BLOCK_SIZE - block_offset;

        if (read_size > bytes_to_read - bytes_read) {
            read_size = bytes_to_read - bytes_read;
        }

        memcpy(
            buffer + bytes_read,
            Disk + cur * BLOCK_SIZE + block_offset,
            read_size
        );

        bytes_read += read_size;
        block_offset = 0;

        if (bytes_read < bytes_to_read) {
            cur = FAT[cur];
        }
    }

    buffer[bytes_read] = '\0';
    desc->offset += bytes_read;

    pthread_mutex_unlock(&fs_mutex);

    return bytes_read;
}

//删除文件
bool fs_delete(const char *filename) {
    for (int i = 0; i < MAX_ENTRY; i++) {
        if (Directory[i].ac == 1 && strcmp(Directory[i].name, filename) == 0) {

            // 先释放原来的文件内容,全部标记为0         
             int cur = Directory[i].start_block;
             while (cur != -1) {
                 int next = FAT[cur];
                 FAT[cur] = 0;

                 if (next == -1) {
                     break;
                 }

                 cur = next;
             }
            Directory[i].ac = 0; // 标记为未使用
            printf("File deleted successfully: %s\n", filename);
            return true;
        }
    }

    printf("Failed to delete file: %s\n", filename);
    return false;
}
bool fs_seek(int pid, int fd, int offset)
{
    if (pid < 0 || pid >= MAX_PROCESS ||
        fd < 0 || fd >= MAX_FD ||
        offset < 0) {
        return false;
    }

    if (ProcessTable[pid].fd_table[fd].used == 0) {
        return false;
    }

    ProcessTable[pid].fd_table[fd].offset = offset;
    return true;
}