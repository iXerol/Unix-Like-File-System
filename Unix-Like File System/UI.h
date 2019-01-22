#ifndef UI_h
#define UI_h

#include "commands.h"
#include <assert.h>

#ifdef _WIN32

#include <conio.h>        //windows中用于不回显字符

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
#endif

void UI_login(void);
void UI_command(void);

void UI_login() {
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
                printf("\n\n");
                break;
            }
            else {
                putchar('*');
                password[i++] = ch;
            }
        }

        struct inode_t* passwd = find_file_by_path(current_working_inode, "/etc/passwd");
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

                UI_command();
            }
        }

    } while (true);

}

void UI_command() {

}






#endif /* UI_h */
