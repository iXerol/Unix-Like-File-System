#ifndef functions_h
#define functions_h

#include "variables.h"
#include "strings.h"
#include <stdlib.h>
#include <stdbool.h>

void read_data(struct inode_t* inode, char* data);

void erase_data(struct inode_t* inode);

void write_data(struct inode_t* inode, char* data, size_t size);

struct inode_t* get_inode_by_num(unsigned int n);

unsigned int get_free_inode(void);

unsigned int get_free_data_block(void);

void return_inode(unsigned int n);

void return_data_block(unsigned int n);

bool is_owner(struct inode_t* file);

bool check_read_permission(struct inode_t* file);

bool check_write_permission(struct inode_t* file);

bool check_execute_permission(struct inode_t* file);

bool is_descendant_directory(struct inode_t* ancestor, struct inode_t* descendant);

struct inode_t* find_file_from_parent(struct inode_t* directory, char* filename);

struct inode_t* find_file_by_path(struct inode_t* current_inode, const char* path);

struct inode_t* find_parent(struct inode_t* working_directory, char* path);

#endif /* functions_h */
