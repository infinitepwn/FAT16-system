#include "FAT.h"

int fs_open(const char *filename, int mode)
{
    if (filename == NULL) {
        return -1;
    }

    // 在当前目录下找普通文件
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

    // 找空闲 fd
    for (int fd = 0; fd < MAX_FD; fd++) {
        if (FDTable[fd].used == 0) {
            FDTable[fd].used = 1;
            FDTable[fd].dir_index = dir_index;
            FDTable[fd].offset = 0;
            FDTable[fd].mode = mode;

            printf("open success: %s, fd = %d\n", filename, fd);
            return fd;
        }
    }

    printf("open failed: too many open files\n");
    return -1;
}

bool fs_close(int fd)
{
    if (fd < 0 || fd >= MAX_FD) {
        printf("close failed: invalid fd\n");
        return false;
    }

    if (FDTable[fd].used == 0) {
        printf("close failed: fd is not open\n");
        return false;
    }

    FDTable[fd].used = 0;
    FDTable[fd].dir_index = -1;
    FDTable[fd].offset = 0;
    FDTable[fd].mode = 0;

    printf("close success: fd = %d\n", fd);
    return true;
}
bool fs_seek(int fd, int offset)
{
    if (fd < 0 || fd >= MAX_FD) {
        return false;
    }

    if (FDTable[fd].used == 0) {
        return false;
    }

    if (offset < 0) {
        return false;
    }

    FDTable[fd].offset = offset;
    return true;
}