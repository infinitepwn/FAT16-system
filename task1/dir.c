#include "FAT.h"
//mkdir
bool mkdir(const char *dirname) {
    for (int i = 0; i < MAX_ENTRY; i++) {
        if (Directory[i].ac == 0) { //找到空闲目录项
            strncpy(Directory[i].name, dirname, 16);
            Directory[i].size = 0;
            Directory[i].start_block = -1;  
            Directory[i].ac = 1; //标记为已使用
            Directory[i].type = 1; //1表示目录
            Directory[i].parent = current_dir; //设置父目录索引
            return true;
        }
    }
    return false; //目录创建失败
}
int get_dir_size(int dir_index)
{
    int total = 0;

    for (int i = 0; i < MAX_ENTRY; i++) {
        if (Directory[i].ac == 1 && Directory[i].parent == dir_index) {
            if (Directory[i].type == 0) {
                // 普通文件
                total += Directory[i].size;
            } else if (Directory[i].type == 1) {
                // 子目录，递归统计
                total += get_dir_size(i);
            }
        }
    }

    return total;
}
//ls
bool ls()
{
    for (int i = 0; i < MAX_ENTRY; i++) {
        if (Directory[i].ac == 1 && Directory[i].parent == current_dir) {

            int show_size;

            if (Directory[i].type == 1) {
                show_size = get_dir_size(i);
            } else {
                show_size = Directory[i].size;
            }

            printf("%s\t%s\t%d bytes\n",
                   Directory[i].type == 1 ? "DIR" : "FILE",
                   Directory[i].name,
                   show_size);
        }
    }

    return true;
}

bool cd(const char *dirname)
{
    if (dirname == NULL) {
        return false;
    }

    // 回到根目录
    if (strcmp(dirname, "/") == 0) {
        current_dir = 0;
        return true;
    }

    // 回到父目录
    if (strcmp(dirname, "..") == 0) {
        if (Directory[current_dir].parent != -1) {
            current_dir = Directory[current_dir].parent;
        }
        return true;
    }

    // 进入当前目录下的子目录
    for (int i = 0; i < MAX_ENTRY; i++) {
        if (Directory[i].ac == 1 &&
            Directory[i].parent == current_dir &&
            Directory[i].type == 1 &&
            strcmp(Directory[i].name, dirname) == 0) {
            current_dir = i;
            return true;
        }
    }

    return false;
}
