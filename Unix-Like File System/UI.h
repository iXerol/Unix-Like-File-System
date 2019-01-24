#ifndef UI_h
#define UI_h

#include "commands.h"
#include <assert.h>

#ifdef _WIN32

#include <conio.h>        //windows中用于不回显字符

void UI_clear() {
    system("cls");
}                            //在 Windows 中调用 cls 命令清屏

#else

#include<termios.h>   //*nix下用于自定义不回显字符
#include<unistd.h>    //*nix下用于自定义不回显字符
int getch()
{
    int c = 0;
    struct termios org_opts, new_opts;
    int res = 0;
    //-----  store old settings -----------
    res = tcgetattr(STDIN_FILENO, &org_opts);
    assert(res == 0);
    //---- set new terminal parms --------
    memcpy(&new_opts, &org_opts, sizeof(new_opts));
    new_opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOKE | ICRNL);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_opts);
    c = getchar();
    //------  restore old settings ---------
    res = tcsetattr(STDIN_FILENO, TCSANOW, &org_opts);
    assert(res == 0);
    if(c == '\n') c = '\r';
    else if(c == 127) c = '\b';
    return c;
}

void UI_clear() {
    system("clear");
}                            //在 *nix 中调用 clear 命令清屏

#endif

void UI_create_user(void);
void UI_change_password(void);
void UI_login(void);
void UI_command(void);

void UI_create_user() {
    int i;
    char username[USER_NAME_LENGTH * 4];
    char password1[USER_PASSWORD_LENGTH * 4];
    char password2[USER_PASSWORD_LENGTH * 4];
    char group[GROUP_NAME_LENGTH * 4];

    printf("Username:");
    i = 0;
    while (1) {
        char ch;
        ch = getch();
        if (ch == '\b') {
            if (i != 0) {
                printf("\b \b");
                i--;
            }
            else {
                username[i] = '\0';
            }
        }
        else if (ch == '\r' || ch == '\n') {
            username[i] = '\0';
            printf("\n");
            break;
        }
        else {
            putchar('*');
            username[i++] = ch;
        }
    }

    printf("Password:");
    i = 0;
    while (1) {
        char ch;
        ch = getch();
        if (ch == '\b') {
            if (i != 0) {
                printf("\b \b");
                i--;
            }
            else {
                password1[i] = '\0';
            }
        }
        else if (ch == '\r' || ch == '\n') {
            password1[i] = '\0';
            printf("\n");
            break;
        }
        else {
            putchar('*');
            password1[i++] = ch;
        }
    }

    printf("Retype Password:");
    i = 0;
    while (1) {
        char ch;
        ch = getch();
        if (ch == '\b') {
            if (i != 0) {
                printf("\b \b");
                i--;
            }
            else {
                password2[i] = '\0';
            }
        }
        else if (ch == '\r' || ch == '\n') {
            password2[i] = '\0';
            printf("\n");
            break;
        }
        else {
            putchar('*');
            password2[i++] = ch;
        }
    }

    printf("Group:");
    i = 0;
    while (1) {
        char ch;
        ch = getch();
        if (ch == '\b') {
            if (i != 0) {
                printf("\b \b");
                i--;
            }
            else {
                group[i] = '\0';
            }
        }
        else if (ch == '\r' || ch == '\n') {
            group[i] = '\0';
            printf("\n");
            break;
        }
        else {
            putchar('*');
            group[i++] = ch;
        }
    }
    if (strcmp(password1, password2) != 0) {
        printf("useradd: try again\n");
    } else {
        create_user(username, password1, group);
    }
}

void UI_change_password() {
    int i;
    char old_password[USER_PASSWORD_LENGTH * 4];
    char new_password1[USER_PASSWORD_LENGTH * 4];
    char new_password2[USER_PASSWORD_LENGTH * 4];

    printf("Changing password for %s.\n", current_user);
    printf("Old Password:");
    i = 0;
    while (1) {
        char ch;
        ch = getch();
        if (ch == '\b') {
            if (i != 0) {
                printf("\b \b");
                i--;
            }
            else {
                old_password[i] = '\0';
            }
        }
        else if (ch == '\r' || ch == '\n') {
            old_password[i] = '\0';
            printf("\n");
            break;
        }
        else {
            putchar('*');
            old_password[i++] = ch;
        }
    }

    printf("New Password:");
    i = 0;
    while (1) {
        char ch;
        ch = getch();
        if (ch == '\b') {
            if (i != 0) {
                printf("\b \b");
                i--;
            }
            else {
                new_password1[i] = '\0';
            }
        }
        else if (ch == '\r' || ch == '\n') {
            new_password1[i] = '\0';
            printf("\n");
            break;
        }
        else {
            putchar('*');
            new_password1[i++] = ch;
        }
    }

    printf("Retype New Password:");
    i = 0;
    while (1) {
        char ch;
        ch = getch();
        if (ch == '\b') {
            if (i != 0) {
                printf("\b \b");
                i--;
            }
            else {
                new_password2[i] = '\0';
            }
        }
        else if (ch == '\r' || ch == '\n') {
            new_password2[i] = '\0';
            printf("\n");
            break;
        }
        else {
            putchar('*');
            new_password2[i++] = ch;
        }
    }
    if (strcmp(new_password1, new_password2) != 0) {
        printf("passwd: try again\n");
    } else {
        change_password(old_password, new_password1);
    }
}

void UI_login() {
    struct inode_t* passwd = find_file_by_path(current_working_inode, "/etc/passwd");
    if (passwd == NULL) {
        printf("No user exists, now create one.\n");
        UI_create_user();
    }
    char username[USER_NAME_LENGTH];
    char password[USER_PASSWORD_LENGTH * 4];
    do {
        memset(username, 0, USER_NAME_LENGTH);
        memset(password, 0, USER_PASSWORD_LENGTH * 4);
        printf("login: ");
        scanf("%s", username);
        printf("password: ");
        char c;
        scanf("%c", &c);
        int i = 0;
        while (1) {
            char ch;
            ch = getch();
            if (ch == '\b') {
                if (i != 0) {
                    printf("\b \b");
                    i--;
                }
                else {
                    password[i] = '\0';
                }
            }
            else if (ch == '\r' || ch == '\n') {
                password[i] = '\0';
                printf("\n");
                break;
            }
            else {
                putchar('*');
                password[i++] = ch;
            }
        }

        char* user_data = malloc(passwd->size);
        read_data(passwd, user_data);
        char tmp_username[USER_NAME_LENGTH];
        char tmp_password[USER_PASSWORD_LENGTH];
        char tmp_group[GROUP_NAME_LENGTH];

        for (size_t i = 0; i < passwd->size / USER_DATA_LENGTH; ++i) {
            memset(tmp_username, 0, USER_NAME_LENGTH);
            memset(tmp_password, 0, USER_PASSWORD_LENGTH);
            memset(tmp_group, 0, GROUP_NAME_LENGTH);
            strncpy(tmp_username, user_data + i * USER_DATA_LENGTH, USER_NAME_LENGTH);
            strncpy(tmp_password, user_data + i * USER_DATA_LENGTH + USER_NAME_LENGTH, USER_PASSWORD_LENGTH);
            strncpy(tmp_group, user_data + i * USER_DATA_LENGTH + USER_NAME_LENGTH + USER_PASSWORD_LENGTH, GROUP_NAME_LENGTH);
            if (strcmp(username, tmp_username) == 0 && strcmp(password, tmp_password) == 0) {
                strcpy(current_user, username);
                strcpy(current_group, tmp_group);

                char user_directory[USER_NAME_LENGTH + 8] = "/home/";
                strcat(user_directory, username);
                cd(user_directory);
                
                return;
            }
        }

        printf("Login incorrect\n");
    } while (true);
}

void UI_command() {
    bool is_super_user = strcmp(current_user, "root") == 0;
    char command[MAX_COMMAND_LENGTH], parameters[MAX_COMMAND_LENGTH];
    char first_parameter[MAX_COMMAND_LENGTH], second_parameter[MAX_COMMAND_LENGTH], other_parameters[MAX_COMMAND_LENGTH];
    while (true) {
        printf("ULFS:%s %s%c ", current_working_directory, current_user, is_super_user ? '#' : '$');
        fgets(command, MAX_COMMAND_LENGTH, stdin);
        if (start_with(command, "useradd", parameters)) {
            if (is_super_user) {
                UI_create_user();
            } else {
                printf("useradd: Permission denied\n");
            }
        } else if (start_with(command, "passwd", parameters)) {
            UI_change_password();
        } else if (start_with(command, "login", parameters) || start_with(command, "logout", parameters)) {
            save();
            break;
        } else if(start_with(command, "exit", parameters)) {
            save();
            fclose(disk);
            exit(0);
        } else if (start_with(command, "show", parameters)) {
            show();
        } else if (start_with(command, "pwd", parameters)) {
            present_working_directory();
        } else if (start_with(command, "umask", parameters)) {
            split_parameters(parameters, first_parameter, other_parameters);
            if (strcmp(first_parameter, "") == 0) {
                show_umask();
            } else {
                unsigned short new_umask = string_to_octal(first_parameter);
                change_umask(new_umask);
            }
        } else if (start_with(command, "cd", parameters)) {
            split_parameters(parameters, first_parameter, other_parameters);
            if (strcmp(first_parameter, "") == 0) {
                cd("/");
            } else {
                cd(first_parameter);
            }
        } else if (start_with(command, "ls", parameters)) {
            split_parameters(parameters, first_parameter, other_parameters);
            if (strcmp(first_parameter, "") == 0) {
                list(".");
            } else {
                list(first_parameter);
            }
        } else if (start_with(command, "stat", parameters)) {
            split_parameters(parameters, first_parameter, other_parameters);
            if (strcmp(first_parameter, "") == 0) {
                printf("usage: stat file\n");
            } else {
                status(current_working_inode, first_parameter);
            }
        } else if (start_with(command, "mkdir", parameters)) {
            split_parameters(parameters, first_parameter, other_parameters);
            if (strcmp(first_parameter, "") == 0) {
                printf("usage: mkdir directory\n");
            } else {
                create_directory(current_working_inode, first_parameter);
            }
        } else if (start_with(command, "rmdir", parameters)) {
            split_parameters(parameters, first_parameter, other_parameters);
            if (strcmp(first_parameter, "") == 0) {
                printf("usage: rmdir directory\n");
            } else {
                rmdir(first_parameter);
            }
        } else if (start_with(command, "ln", parameters)) {
            split_parameters(parameters, first_parameter, other_parameters);
            strcpy(parameters, other_parameters);
            split_parameters(parameters, second_parameter, other_parameters);
            if (strcmp(second_parameter, "") == 0) {
                printf("usage: ln source_file target_file\n");
            } else {
                link_file(current_working_inode, first_parameter, second_parameter);
            }
        } else if (start_with(command, "cp", parameters)) {
            split_parameters(parameters, first_parameter, other_parameters);
            strcpy(parameters, other_parameters);
            split_parameters(parameters, second_parameter, other_parameters);
            if (strcmp(second_parameter, "") == 0) {
                printf("usage: cp source_file target_file\n");
            } else {
                copy_file(first_parameter, second_parameter);
            }
        } else if (start_with(command, "mv", parameters)) {
            split_parameters(parameters, first_parameter, other_parameters);
            strcpy(parameters, other_parameters);
            split_parameters(parameters, second_parameter, other_parameters);
            if (strcmp(second_parameter, "") == 0) {
                printf("usage: mv source_file target_file\n");
            } else {
                move_file(first_parameter, second_parameter);
            }
        } else if (start_with(command, "rm", parameters)) {
            split_parameters(parameters, first_parameter, other_parameters);
            if (strcmp(first_parameter, "") == 0) {
                printf("usage: rm file\n");
            } else {
                remove_regular_file(first_parameter);
            }
        } else if (start_with(command, "touch", parameters)) {
            split_parameters(parameters, first_parameter, other_parameters);
            if (strcmp(first_parameter, "") == 0) {
                printf("usage: touch file\n");
            } else {
                touch_file(first_parameter);
            }
        } else if (start_with(command, "edit", parameters)) {
            split_parameters(parameters, first_parameter, other_parameters);
            strcpy(parameters, other_parameters);
            split_parameters(parameters, second_parameter, other_parameters);
            if (strcmp(second_parameter, "") == 0) {
                printf("usage: edit file size\n");
            } else {
                struct inode_t* file = find_file_by_path(current_working_inode, first_parameter);
                if (file == NULL) {
                    printf("edit: %s: No such file or directory\n", first_parameter);
                } else if ((file->mode & 07000) != ISREG){
                    printf("edit: %s: Not a regular file.\n", first_parameter);
                } else {
                    size_t new_size = atoi(second_parameter);
                    resize_text_file(file, new_size);
                }
            }
        } else if (start_with(command, "cat", parameters)) {
            split_parameters(parameters, first_parameter, other_parameters);
            if (strcmp(first_parameter, "") == 0) {
                printf("usage: cat file\n");
            } else {
                cat(first_parameter);
            }
        } else if (start_with(command, "chmod", parameters)) {
            split_parameters(parameters, first_parameter, other_parameters);
            strcpy(parameters, other_parameters);
            split_parameters(parameters, second_parameter, other_parameters);
            if (strcmp(second_parameter, "") == 0) {
                printf("usage: chmod mode file\n");
            } else {
                size_t i;
                for (i = 0; i < strlen(first_parameter); ++i) {
                    if (first_parameter[i] < '0' || first_parameter[i] > '7') {
                        printf("chmod: Invalid file mode: %s\n", first_parameter);
                        break;
                    }
                }
                if (i == strlen(first_parameter)) {
                    unsigned short privilege = string_to_octal(first_parameter);
                    change_mode(second_parameter, privilege);
                }
            }
        } else if (start_with(command, "chown", parameters)) {
            split_parameters(parameters, first_parameter, other_parameters);
            strcpy(parameters, other_parameters);
            split_parameters(parameters, second_parameter, other_parameters);
            if (strcmp(second_parameter, "") == 0) {
                printf("usage: chown owner file\n");
            } else {
                change_owner(second_parameter, first_parameter);
            }
        } else if (start_with(command, "chgrp", parameters)) {
            split_parameters(parameters, first_parameter, other_parameters);
            strcpy(parameters, other_parameters);
            split_parameters(parameters, second_parameter, other_parameters);
            if (strcmp(second_parameter, "") == 0) {
                printf("usage: chgrp group file\n");
            } else {
                change_group(second_parameter, first_parameter);
            }
        } else {
            printf("Command not found\n");
        }
        save();
    }
}






#endif /* UI_h */
