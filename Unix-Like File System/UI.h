//
//  UI.h
//  Unix-Like File System
//
//  Created by Xerol Wong on 1/23/19.
//  Copyright © 2019 Xerol Wong. All rights reserved.
//

#ifndef UI_h
#define UI_h

#include "commands.h"

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










#endif /* UI_h */
