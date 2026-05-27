#include "FAT.h"
#include <string.h>

char Disk[DISK_MAXLEN];
int FAT[BLOCK_NUM];
DirEntry Directory[MAX_ENTRY];
int current_dir = 0;
FileDescriptor FDTable[MAX_FD];
pthread_mutex_t fs_mutex = PTHREAD_MUTEX_INITIALIZER;
PCB ProcessTable[MAX_PROCESS];

void init_process_table(void)
{
    memset(ProcessTable, 0, sizeof(ProcessTable));

    for (int p = 0; p < MAX_PROCESS; p++) {
        ProcessTable[p].pid = p;

        for (int fd = 0; fd < MAX_FD; fd++) {
            ProcessTable[p].fd_table[fd].dir_index = -1;
            ProcessTable[p].fd_table[fd].offset = 0;
            ProcessTable[p].fd_table[fd].mode = 0;
            ProcessTable[p].fd_table[fd].used = 0;
        }
    }
}
void init_fs(void)
{
    memset(Disk, 0, sizeof(Disk));
    memset(FAT, 0, sizeof(FAT));
    memset(Directory, 0, sizeof(Directory));

    for (int i = 0; i < MAX_ENTRY; i++) {
        Directory[i].start_block = -1;
        Directory[i].parent = -1;
        Directory[0].reader_count = 0;
        Directory[0].writer_pid = -1;
    }

    strcpy(Directory[0].name, "/");
    Directory[0].size = 0;
    Directory[0].start_block = -1;
    Directory[0].ac = 1;
    Directory[0].type = 1;
    Directory[0].parent = -1;
    Directory[0].reader_count = 0;
    Directory[0].writer_pid = -1;

    current_dir = 0;

    init_process_table();
}