#ifndef UI_h
#define UI_h

#include "file.h"
#include <assert.h>

void UI_clear(void);
int getch(void);

void UI_create_user(void);
void UI_change_password(void);
void UI_login(void);
void UI_command(void);

#endif /* UI_h */
