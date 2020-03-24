#ifndef commands_h
#define commands_h

#include "functions.h"

void show(void);

void create_user(char* username, char* password, char* group);

void change_password(char* old_password, char* new_password);

void change_mode(char* filename, unsigned short privilege);

void change_owner(char* path, char* user);

void change_group(char* path, char* group);

void present_working_directory(void);

void list(char* path);

void status(struct inode_t* directory, char* path);

void cd(char* path);

void copy_file(char* source_file_path, char* target_file_path);

void move_file(char* source_file_path, char* target_file_path);

void link_file(struct inode_t* working_directory, char* target_file_path, char* source_file_path);

void create_directory(struct inode_t* working_directory, char* directory_path);

void remove_directory(char* path);

void touch_file(char* path);

void resize_text_file(struct inode_t* inode, size_t new_size);

void remove_regular_file(char* path);

void cat(char* path);

void show_umask(void);

void change_umask(unsigned short new_umask);

#endif /* commands_h */
