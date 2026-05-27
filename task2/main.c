#include "FAT.h"

void print_path_recursive(int dir)
{
    if (dir == 0) {
        printf("/");
        return;
    }

    int parent = Directory[dir].parent;

    print_path_recursive(parent);

    if (parent != 0) {
        printf("/");
    }

    printf("%s", Directory[dir].name);
}

void print_prompt()
{
    printf("miniFS:");
    print_path_recursive(current_dir);
    printf("$ ");
}

int main()
{
    char line[256];
    char cmd[32];
    char arg1[64];
    char arg2[128];
    char buffer[1024];

    init_fs();

    while (1) {
        print_prompt();

        if (fgets(line, sizeof(line), stdin) == NULL) {
            break;
        }

        line[strcspn(line, "\n")] = '\0';

        if (strlen(line) == 0) {
            continue;
        }

        cmd[0] = '\0';
        arg1[0] = '\0';
        arg2[0] = '\0';

        sscanf(line, "%31s %63s %127[^\n]", cmd, arg1, arg2);

        if (strcmp(cmd, "exit") == 0) {
            break;
        }

        else if (strcmp(cmd, "ls") == 0) {
            ls();
        }

        else if (strcmp(cmd, "mkdir") == 0) {
            if (strlen(arg1) == 0) {
                printf("usage: mkdir dirname\n");
            } else if (!mkdir(arg1)) {
                printf("mkdir failed: %s\n", arg1);
            }
        }

        else if (strcmp(cmd, "cd") == 0) {
            if (strlen(arg1) == 0) {
                printf("usage: cd dirname\n");
            } else if (!cd(arg1)) {
                printf("cd failed: %s\n", arg1);
            }
        }

        else if (strcmp(cmd, "touch") == 0) {
            if (strlen(arg1) == 0) {
                printf("usage: touch filename\n");
            } else {
                fs_create(arg1);
            }
        }

        else if (strcmp(cmd, "rm") == 0) {
            if (strlen(arg1) == 0) {
                printf("usage: rm filename\n");
            } else {
                fs_delete(arg1);
            }
        }

        /*
         * open pid filename mode
         * 例如：
         * open 0 a.txt rw
         */
        else if (strcmp(cmd, "open") == 0) {
            int pid;
            char filename[64];
            char mode_str[16];

            if (sscanf(line, "%31s %d %63s %15s", cmd, &pid, filename, mode_str) != 4) {
                printf("usage: open pid filename r|w|rw\n");
                continue;
            }

            int mode;

            if (strcmp(mode_str, "r") == 0) {
                mode = MODE_READ;
            } else if (strcmp(mode_str, "w") == 0) {
                mode = MODE_WRITE;
            } else if (strcmp(mode_str, "rw") == 0) {
                mode = MODE_RDWR;
            } else {
                printf("invalid mode: %s\n", mode_str);
                continue;
            }

            fs_open(pid, filename, mode);
        }

        /*
         * close pid fd
         * 例如：
         * close 0 0
         */
        else if (strcmp(cmd, "close") == 0) {
            int pid;
            int fd;

            if (sscanf(line, "%31s %d %d", cmd, &pid, &fd) != 3) {
                printf("usage: close pid fd\n");
                continue;
            }

            fs_close(pid, fd);
        }

        /*
         * write pid fd content
         * 例如：
         * write 0 0 hello world
         */
        else if (strcmp(cmd, "write") == 0) {
            int pid;
            int fd;
            char content[128];

            if (sscanf(line, "%31s %d %d %127[^\n]", cmd, &pid, &fd, content) != 4) {
                printf("usage: write pid fd content\n");
                continue;
            }

            fs_write(pid, fd, content, strlen(content));
        }

        /*
         * read pid fd size
         * 例如：
         * read 0 0 5
         */
        else if (strcmp(cmd, "read") == 0) {
            int pid;
            int fd;
            int size;

            if (sscanf(line, "%31s %d %d %d", cmd, &pid, &fd, &size) != 4) {
                printf("usage: read pid fd size\n");
                continue;
            }

            if (size >= (int)sizeof(buffer)) {
                size = sizeof(buffer) - 1;
            }

            int n = fs_read(pid, fd, buffer, size);

            if (n >= 0) {
                buffer[n] = '\0';
                printf("%s\n", buffer);
            }
        }

        /*
         * seek pid fd offset
         * 例如：
         * seek 0 0 0
         */
        else if (strcmp(cmd, "seek") == 0) {
            int pid;
            int fd;
            int offset;

            if (sscanf(line, "%31s %d %d %d", cmd, &pid, &fd, &offset) != 4) {
                printf("usage: seek pid fd offset\n");
                continue;
            }

            if (!fs_seek(pid, fd, offset)) {
                printf("seek failed\n");
            }
        }

        else if (strcmp(cmd, "help") == 0) {
            printf("commands:\n");
            printf("  ls\n");
            printf("  mkdir dirname\n");
            printf("  cd dirname\n");
            printf("  touch filename\n");
            printf("  rm filename\n");
            printf("  open pid filename r|w|rw\n");
            printf("  close pid fd\n");
            printf("  write pid fd content\n");
            printf("  read pid fd size\n");
            printf("  seek pid fd offset\n");
            printf("  exit\n");
        }

        else {
            printf("unknown command: %s\n", cmd);
            printf("type 'help' for help\n");
        }
    }

    return 0;
}