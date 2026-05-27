/*系统是基于内存的：建⽴⼀个数组：把这个数组当成硬盘，实现⽂件系统。
假设只有⼀个进程使⽤。*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#define DISK_MAXLEN 2560
#define BLOCK_SIZE 64
#define BLOCK_NUM (DISK_MAXLEN / BLOCK_SIZE)
#define MAX_ENTRY 32
#define MAX_OPEN_FILE 32
#define MAX_FD 16
#define MAX_PROCESS 8
#define MODE_READ  1
#define MODE_WRITE 2
#define MODE_RDWR  3
char Disk[DISK_MAXLEN];
int FAT[BLOCK_NUM]; //FAT表：记录每个块的状态，0表示空闲，-1表示文件结束，其他正数表示下一个块的索引。
int current_dir; //当前目录索引，初始为根目录


typedef struct {
    char name[16];
    int size;
    int start_block;
    int ac;
    int type;
    int parent;

    int reader_count;   // 当前读者数量
    int writer_pid;     // -1 表示没有写者
} DirEntry;

typedef struct {
    int used;       // 0 空闲，1 已使用
    int dir_index;  // 对应 Directory[] 下标
    int offset;     // 当前读写偏移
    int mode;       // 打开模式
} FileDescriptor;

extern char Disk[DISK_MAXLEN];
extern int FAT[BLOCK_NUM];
extern DirEntry Directory[MAX_ENTRY];
extern int current_dir;
extern FileDescriptor FDTable[MAX_FD];
extern pthread_mutex_t fs_mutex;

typedef struct {
    int pid;
    FileDescriptor fd_table[MAX_FD];
} PCB;

extern PCB ProcessTable[MAX_PROCESS];

void init_process_table(void);

/* init */
void init_fs(void);
void init_fd_table(void);

/* directory */
bool fs_mkdir(const char *dirname);
bool fs_ls(void);
bool fs_cd(const char *dirname);

/* file by name */
bool fs_create(const char *filename);
bool fs_delete(const char *filename);
bool read_file(const char *filename, char *buffer);

/* fd operations */
int fs_open(int pid, const char *filename, int mode);
bool fs_close(int pid, int fd);
int fs_read(int pid, int fd, char *buffer, int size);
int fs_write(int pid, int fd, const char *data, int size);
bool fs_seek(int pid, int fd, int offset);

/* prompt */
void print_prompt(void);

bool cd(const char *dirname);
bool ls();
bool mkdir(const char *dirname);