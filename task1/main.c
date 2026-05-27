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

        else if (strcmp(cmd, "open") == 0) {
            if (strlen(arg1) == 0 || strlen(arg2) == 0) {
                printf("usage: open filename mode\n");
                printf("mode: r | w | rw\n");
                continue;
            }

            int mode;

            if (strcmp(arg2, "r") == 0) {
                mode = MODE_READ;
            } else if (strcmp(arg2, "w") == 0) {
                mode = MODE_WRITE;
            } else if (strcmp(arg2, "rw") == 0) {
                mode = MODE_RDWR;
            } else {
                printf("invalid mode: %s\n", arg2);
                continue;
            }

            fs_open(arg1, mode);
        }

        else if (strcmp(cmd, "close") == 0) {
            if (strlen(arg1) == 0) {
                printf("usage: close fd\n");
            } else {
                int fd = atoi(arg1);
                fs_close(fd);
            }
        }

        else if (strcmp(cmd, "write") == 0) {
            if (strlen(arg1) == 0 || strlen(arg2) == 0) {
                printf("usage: write fd content\n");
            } else {
                int fd = atoi(arg1);
                fs_write(fd, arg2, strlen(arg2));
            }
        }

        else if (strcmp(cmd, "read") == 0) {
            if (strlen(arg1) == 0 || strlen(arg2) == 0) {
                printf("usage: read fd size\n");
            } else {
                int fd = atoi(arg1);
                int size = atoi(arg2);

                int n = fs_read(fd, buffer, size);

                if (n >= 0) {
                    buffer[n] = '\0';
                    printf("%s\n", buffer);
                }
            }
        }

        else if (strcmp(cmd, "seek") == 0) {
            if (strlen(arg1) == 0 || strlen(arg2) == 0) {
                printf("usage: seek fd offset\n");
            } else {
                int fd = atoi(arg1);
                int offset = atoi(arg2);

                if (!fs_seek(fd, offset)) {
                    printf("seek failed\n");
                }
            }
        }

        else if (strcmp(cmd, "help") == 0) {
            printf("commands:\n");
            printf("  ls\n");
            printf("  mkdir dirname\n");
            printf("  cd dirname\n");
            printf("  touch filename\n");
            printf("  rm filename\n");
            printf("  cat filename\n");
            printf("  open filename r|w|rw\n");
            printf("  close fd\n");
            printf("  write fd content\n");
            printf("  read fd size\n");
            printf("  seek fd offset\n");
            printf("  exit\n");
        }

        else {
            printf("unknown command: %s\n", cmd);
            printf("type 'help' for help\n");
        }
    }

    return 0;
}