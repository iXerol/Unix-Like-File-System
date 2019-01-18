#ifndef functions_h
#define functions_h

#include "inode.h"
#include "variables.h"
#include <string.h>
#include "directory.h"

void initialize_superblock(void);

void initialize_inodes(void);

void initialize_data_block(void);

void write_inode(int n);

void create_root(void);

struct inode_t* get_inode_by_num(unsigned int n);

unsigned int get_free_inode(void);

unsigned int get_free_data_block(void);

void link_file(struct inode_t* working_directory, char* source_file, char* target_file);

struct inode_t* find_file_from_parent(struct inode_t* directory, char *filename);

void create_directory(struct inode_t* parent_directory, char* dictionary_name);


struct inode_t* find_file_by_path(struct inode_t* current_inode, char* path);




void initialize_superblock() {
    superblock.num_free_inode = INODE_NUM;

    superblock.num_free_block = BLOCK_NUM - DATA_START_BLOCK;

    superblock.umask = DEFAULT_UMASK;

    superblock.free_inodes = UINT64_MAX;
    printf("Successfully initialized superblock.\n");
}

void initialize_inodes() {
    memset(inodes, 0, sizeof(struct inode_t) * INODE_NUM);
    for (int i = 0; i < INODE_NUM; ++i) {
        inodes[i].number = i;
    }
    printf("Successfully initialized inodes.\n");
}

void initialize_data_block() {
    size_t stack_size;
    unsigned int free_block_stack[100];
    unsigned int num_data_block = BLOCK_NUM - DATA_START_BLOCK;
    unsigned int i;
    superblock.stack_size = 100;
    for (i = 0; i < 100; ++i) {
        superblock.free_block_stack[i] = i;
    }
    fseek(disk, (superblock.free_block_stack[0] + DATA_START_BLOCK) * BLOCK_SIZE, SEEK_SET);
    while (i < num_data_block) {
        stack_size = (num_data_block - i < 100) ? num_data_block - i : 100;
        for (int j = 0; j < stack_size; ++i, ++j) {
            free_block_stack[j] = i;
        }
        if (stack_size < 100) {
            free_block_stack[stack_size] = free_block_stack[0];
            free_block_stack[0] = 0;
            ++stack_size;
        }
        fwrite(&stack_size, sizeof(size_t), 1, disk);
        fwrite(free_block_stack, sizeof(unsigned int), stack_size, disk);
        if (free_block_stack[0] != 0) {
            fseek(disk, (free_block_stack[0] + DATA_START_BLOCK) * BLOCK_SIZE, SEEK_SET);
        }
    }
    printf("Successfully initialized datablocks.\n");
}

void write_inode(int n) {
    fseek(disk, INODE_START_BLOCK * BLOCK_SIZE + n * INODE_SIZE, SEEK_SET);
    fwrite(inodes + n, sizeof(struct inode_t), 1, disk);
}

void create_root() {
    //inodes[0] 對應根目錄
    inodes[0].link_count = 1;
    strcpy(inodes[0].user, "system");
    strcpy(inodes[0].group, "system");
    inodes[0].mode = 01644;
    superblock.free_inodes = superblock.free_inodes ^ (1 << 0);

    link_file(get_inode_by_num(0), ".", "/");
    link_file(get_inode_by_num(0), "..", "/");

    printf("Successfully created root directory.\n");
}

struct inode_t* get_inode_by_num(unsigned int n) {
    struct inode_t* inode = NULL;
    fseek(disk, BLOCK_SIZE * 3 + INODE_SIZE * n, SEEK_SET);
    fread(inode, sizeof(struct inode_t), 1, disk);
    return inode;
}

unsigned int get_free_inode() {
    for (unsigned int i = 0; i < INODE_NUM; ++i) {
        if ((superblock.free_inodes & (1 << i)) != 0 ) {
            return i;
        }
    }
    return INODE_NUM;   //inode 數量不足
}

unsigned int get_free_data_block() {
    unsigned int free_data_block;
    if (superblock.stack_size > 1) {
        --superblock.stack_size;
        free_data_block = superblock.free_block_stack[superblock.stack_size];
    } else if (superblock.free_block_stack[0] != 0){
        fseek(disk, (superblock.free_block_stack[0] + DATA_START_BLOCK) * BLOCK_SIZE, SEEK_SET);
        fread(&superblock.stack_size, sizeof(size_t), 1, disk);
        fread(superblock.free_block_stack, sizeof(unsigned int), superblock.stack_size, disk);

        --superblock.stack_size;
        free_data_block = superblock.free_block_stack[superblock.stack_size];
    } else {
        return BLOCK_NUM;   //磁盤容量不足
    }
    return free_data_block;
}


void link_file(struct inode_t* working_directory, char* source_file, char* target_file) {
    if (find_file_by_path(working_directory, source_file)) {
        printf("ln: %s: No such file or directory", source_file);
        return;
    } else if (find_file_from_parent(working_directory, target_file) != NULL) {
        printf("ln: %s: File exists\n", target_file);
        return;
    }
    unsigned short writing_disk = (unsigned short)(working_directory->size / BLOCK_SIZE);
    size_t writing_position =working_directory->size % BLOCK_SIZE;
    if (writing_disk < NADDR - 2) {
        fseek(disk, (working_directory->data_address[writing_disk] + DATA_START_BLOCK) * BLOCK_SIZE + writing_position, SEEK_SET);
    }
}


struct inode_t *find_file_from_parent(struct inode_t* directory, char *filename) {
    struct child_file_t child_file;
    if (directory->size < (NADDR - 2) * BLOCK_SIZE) {
        for (int i = 2; i < (directory->size / sizeof(size_t)); ++i) {
            fseek(disk, (directory->data_address[i * sizeof(size_t) / BLOCK_SIZE] + DATA_START_BLOCK) * BLOCK_SIZE + (i * sizeof(size_t) % BLOCK_SIZE), SEEK_SET);
            fread(&child_file, sizeof(unsigned short), 1, disk);
            if (strcmp(child_file.filename, filename) == 0) {
                return get_inode_by_num(child_file.inode_number);
            }
        }
    }


/* 需要用到一級/二級映射時*/


    return NULL;
}

void create_directory(struct inode_t *parent_directory, char *dictionary_name) {


}

struct inode_t* find_file_by_path(struct inode_t* working_directory, char* path) {
    int index = 0, length = 0;
    if (path[0] == '/') {
        working_directory = get_inode_by_num(0);
        index = 1;
    }
    while (path[index + length] != '/') {

    }

    
    return find_file_from_parent(working_directory, path);
}

struct inode_t* find_parent(struct inode_t* working_directory, char* path) {
    char* lastSlash = strrchr(path, '/');
    if (lastSlash) {
        char parent_path[strlen(lastSlash)];
        strncpy(parent_path, path, strlen(path) - strlen(lastSlash));
        return find_file_by_path(working_directory, parent_path);
    } else {
        return working_directory;
    }
}

#endif /* functions_h */
