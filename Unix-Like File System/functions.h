#ifndef functions_h
#define functions_h

#include "variables.h"
#include "path.h"
#include <stdlib.h>
#include <stdbool.h>

void read_data(struct inode_t* inode, char* data);

void erase_data(struct inode_t* inode);

void write_data(struct inode_t* inode, char* data);

struct inode_t* get_inode_by_num(unsigned int n);

unsigned int get_free_inode(void);

unsigned int get_free_data_block(void);

void return_inode(unsigned int n);

void return_data_block(unsigned int n);

bool check_read_permission(struct inode_t* file);

bool check_write_permission(struct inode_t* file);

bool check_execute_permission(struct inode_t* file);

struct inode_t* find_file_from_parent(struct inode_t* directory, char* filename);

struct inode_t* find_file_by_path(struct inode_t* current_inode, const char* path);

struct inode_t* find_parent(struct inode_t* working_directory, char* path);


void read_data(struct inode_t* inode, char* data) {
    if (inode == NULL || data == NULL) {
        return;
    }
    size_t read_data = 0;

    for (unsigned short i = 0; read_data < MAX_DIRECT_FILE_SIZE && read_data < inode->size; ++i) {
        unsigned int current_data_block = inode->data_address[i];
        size_t size_to_read = inode->size - read_data >= BLOCK_SIZE ? BLOCK_SIZE : inode->size - read_data;
        fseek(disk, (current_data_block + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
        fread(data + read_data, size_to_read, 1, disk);
        read_data += size_to_read;
    }
    if (inode->size == inode->size) {
        return;
    }

    unsigned int level_1_data_block = inode->data_address[NADDR - 2];
    unsigned int level_1_address[NADDR_BLOCK];
    fseek(disk, (level_1_data_block + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
    fread(level_1_address, sizeof(unsigned int), NADDR_BLOCK, disk);
    for (unsigned short i = 0; read_data < MAX_LEVEL_1_FILE_SIZE && read_data < inode->size; ++i) {
        unsigned int current_data_block = level_1_address[i];
        size_t size_to_read = inode->size - read_data >= BLOCK_SIZE ? BLOCK_SIZE : inode->size - read_data;
        fseek(disk, (current_data_block + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
        fread(data + read_data, size_to_read, 1, disk);
        read_data += size_to_read;
    }
    if (inode->size == inode->size) {
        return;
    }

    unsigned int level_2_data_block = inode->data_address[NADDR - 1];
    unsigned int level_2_address[NADDR_BLOCK];
    fseek(disk, (level_2_data_block + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
    fread(level_2_address, sizeof(unsigned int), NADDR_BLOCK, disk);
    for (int i = 0; read_data < MAX_LEVEL_1_FILE_SIZE && read_data < inode->size; ++i) {
        level_1_data_block = level_2_address[i];
        fseek(disk, (level_1_data_block + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
        fread(level_1_address, sizeof(unsigned int), NADDR_BLOCK, disk);
        for (unsigned short j = 0; read_data < MAX_FILE_SIZE && read_data < inode->size; ++j) {
            unsigned int current_data_block = level_1_address[j];
            size_t size_to_read = inode->size - read_data >= BLOCK_SIZE ? BLOCK_SIZE : inode->size - read_data;
            fseek(disk, (current_data_block + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
            fread(data + read_data, size_to_read, 1, disk);
            read_data += size_to_read;
        }
    }
}

void erase_data(struct inode_t* inode) {
    if (inode == NULL) {
        return;
    }
    size_t recovered_data = 0;

    for (unsigned short i = 0; recovered_data < MAX_DIRECT_FILE_SIZE && recovered_data < inode->size; ++i) {
        unsigned int current_data_block = inode->data_address[i];
        size_t size_to_recover = (inode->size - recovered_data >= BLOCK_SIZE) ? BLOCK_SIZE : inode->size - recovered_data;
        return_data_block(current_data_block);
        recovered_data += size_to_recover;
    }
    if (inode->size == inode->size) {
        return;
    }

    unsigned int level_1_data_block = inode->data_address[NADDR - 2];
    unsigned int level_1_address[NADDR_BLOCK];
    fseek(disk, (level_1_data_block + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
    fread(level_1_address, sizeof(unsigned int), NADDR_BLOCK, disk);
    return_data_block(level_1_data_block);
    for (unsigned short i = 0; recovered_data < MAX_LEVEL_1_FILE_SIZE && recovered_data < inode->size; ++i) {
        unsigned int current_data_block = level_1_address[i];
        size_t size_to_recover = inode->size - recovered_data >= BLOCK_SIZE ? BLOCK_SIZE : inode->size - recovered_data;
        return_data_block(current_data_block);
        recovered_data += size_to_recover;
    }
    if (inode->size == inode->size) {
        return;
    }

    unsigned int level_2_data_block = inode->data_address[NADDR - 1];
    unsigned int level_2_address[NADDR_BLOCK];
    fseek(disk, (level_2_data_block + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
    fread(level_2_address, sizeof(unsigned int), NADDR_BLOCK, disk);
    return_data_block(level_2_data_block);
    for (int i = 0; recovered_data < MAX_LEVEL_1_FILE_SIZE && recovered_data < inode->size; ++i) {
        level_1_data_block = level_2_address[i];
        fseek(disk, (level_1_data_block + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
        fread(level_1_address, sizeof(unsigned int), NADDR_BLOCK, disk);
        return_data_block(level_1_data_block);
        for (unsigned short j = 0; recovered_data < MAX_FILE_SIZE && recovered_data < inode->size; ++j) {
            unsigned int current_data_block = level_1_address[j];
            size_t size_to_recover = inode->size - recovered_data >= BLOCK_SIZE ? BLOCK_SIZE : inode->size - recovered_data;
            return_data_block(current_data_block);
            recovered_data += size_to_recover;
        }
    }
}

void write_data(struct inode_t* inode, char* data) {
    if (inode == NULL || data == NULL) {
        return;
    }
    size_t data_size = strlen(data);

    for (unsigned short i = 0; inode->size < MAX_DIRECT_FILE_SIZE && inode->size < data_size; ++i) {
        unsigned int current_data_block = get_free_data_block();
        inode->data_address[i] = current_data_block;
        size_t size_to_write = (data_size - inode->size >= BLOCK_SIZE) ? BLOCK_SIZE : data_size - inode->size;
        fseek(disk, (current_data_block + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
        fwrite(data + inode->size, size_to_write, 1, disk);
        inode->size += size_to_write;
    }
    if (inode->size == data_size) {
        return;
    }

    unsigned int level_1_data_block = get_free_data_block();
    inode->data_address[NADDR - 2] = level_1_data_block;
    unsigned int level_1_address[NADDR_BLOCK];
    for (unsigned short i = 0; inode->size < MAX_LEVEL_1_FILE_SIZE && inode->size < data_size; ++i) {
        unsigned int current_data_block = get_free_data_block();
        level_1_address[i] = current_data_block;
        size_t size_to_write = (data_size - inode->size >= BLOCK_SIZE) ? BLOCK_SIZE : data_size - inode->size;
        fseek(disk, (current_data_block + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
        fwrite(data + inode->size, size_to_write, 1, disk);
        inode->size += size_to_write;
    }
    fseek(disk, (level_1_data_block + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
    fwrite(level_1_address, sizeof(unsigned int), NADDR_BLOCK, disk);
    if (inode->size == data_size) {
        return;
    }

    unsigned int level_2_data_block = get_free_data_block();
    inode->data_address[NADDR - 1] = level_2_data_block;
    unsigned int level_2_address[NADDR_BLOCK];
    for (int i = 0; inode->size < MAX_LEVEL_1_FILE_SIZE && inode->size < data_size; ++i) {
        level_1_data_block = level_2_address[i];
        for (unsigned short j = 0; inode->size < MAX_FILE_SIZE && inode->size < data_size; ++j) {
            unsigned int current_data_block = get_free_data_block();
            level_1_address[j] = current_data_block;
            size_t size_to_write = (data_size - inode->size >= BLOCK_SIZE) ? BLOCK_SIZE : data_size - inode->size;
            fseek(disk, (current_data_block + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
            fwrite(data + inode->size, size_to_write, 1, disk);
            inode->size += size_to_write;
        }
        fseek(disk, (level_1_data_block + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
        fwrite(level_1_address, sizeof(unsigned int), NADDR_BLOCK, disk);
    }
    fseek(disk, (level_2_data_block + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
    fwrite(level_2_address, sizeof(unsigned int), NADDR_BLOCK, disk);
}


struct inode_t* get_inode_by_num(unsigned int n) {
    if (n > INODE_NUM) {
        return NULL;
    }
    return inodes + n;
}

unsigned int get_free_inode() {
    if (superblock.num_free_inode == 0) {
        printf("There is no enough inodes to create file.\n");
        return INODE_NUM;
    }
    for (unsigned int i = 0; i < INODE_NUM; ++i) {
        if ((superblock.free_inodes & (1 << i)) != 0 ) {
            --superblock.num_free_inode;
            superblock.free_inodes ^= (1 << i);
            return i;
        }
    }
    return INODE_NUM;   //inode 數量不足
}

unsigned int get_free_data_block() {
    unsigned int free_data_block;
    if (superblock.stack_size > 1) {
        --superblock.stack_size;
        --superblock.num_free_block;
        free_data_block = superblock.free_block_stack[superblock.stack_size];
    } else if (superblock.stack_size == 1 && superblock.free_block_stack[0] != BLOCK_NUM){
        free_data_block = superblock.free_block_stack[superblock.stack_size];
        --superblock.stack_size;
        --superblock.num_free_block;

        fseek(disk, (superblock.free_block_stack[0] + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
        fread(&superblock.stack_size, sizeof(size_t), 1, disk);
        fread(superblock.free_block_stack, sizeof(unsigned int), superblock.stack_size, disk);
    } else {
        printf("There is no enough space to create file.\n");
        return BLOCK_NUM;   //磁盤容量不足
    }
    return free_data_block;
}

void return_inode(unsigned int n) {
    if (n >= 0 && n < INODE_NUM && (superblock.free_inodes & (1 << n)) == 0) {
        superblock.free_inodes |= (1 << n);
        ++superblock.num_free_inode;
    }
}

void return_data_block(unsigned int n) {
    if (n > BLOCK_NUM - DATA_BLOCK_START) {
        return;
    }
    if (superblock.stack_size == 100) {
        size_t new_stack_size;

        fseek(disk, (superblock.free_block_stack[0] + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
        fread(&new_stack_size, sizeof(size_t), 1, disk);

        if (new_stack_size < 100) {
            ++new_stack_size;
            fseek(disk, (superblock.free_block_stack[0] + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
            fwrite(&new_stack_size, sizeof(size_t), 1, disk);

            fseek(disk, (superblock.free_block_stack[0] + DATA_BLOCK_START) * BLOCK_SIZE + new_stack_size * sizeof(size_t), SEEK_SET);
            fwrite(&n, sizeof(size_t), 1, disk);
        } else {
            new_stack_size= 1;
            unsigned int new_stack[1];
            new_stack[0] = superblock.free_block_stack[0];
            superblock.free_block_stack[0] = n;

            fseek(disk, (n + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
            fwrite(&new_stack_size, sizeof(size_t), 1, disk);
            fwrite(new_stack, sizeof(unsigned int), new_stack_size, disk);
            
        }
    } else if (superblock.stack_size < 100) {
        superblock.free_block_stack[superblock.stack_size] = n;
        ++superblock.stack_size;
        ++superblock.num_free_block;
    }
}




bool check_read_permission(struct inode_t* file) {
    if (file == NULL) {
        return false;
    }
    if (strcmp(file->user, current_user) == 0) {
        //當前用戶為文件主
        return (file->mode & IRUSR);
    } else if (strcmp(file->group, current_group) == 0) {
        //當前用戶組為文件所屬用戶組
        return (file->mode & IRGRP);
    } else {
        //當前用戶為其他用戶
        return (file->mode & IROTH);
    }
}

bool check_write_permission(struct inode_t* file) {
    if (file == NULL) {
        return false;
    }
    if (strcmp(file->user, current_user) == 0) {
        //當前用戶為文件主
        return (file->mode & IWUSR);
    } else if (strcmp(file->group, current_group) == 0) {
        //當前用戶組為文件所屬用戶組
        return (file->mode & IWGRP);
    } else {
        //當前用戶為其他用戶
        return (file->mode & IWOTH);
    }
}

bool check_execute_permission(struct inode_t* file) {
    if (file == NULL) {
        return false;
    }
    if (strcmp(file->user, current_user) == 0) {
        //當前用戶為文件主
        return (file->mode & IXUSR);
    } else if (strcmp(file->group, current_group) == 0) {
        //當前用戶組為文件所屬用戶組
        return (file->mode & IXGRP);
    } else {
        //當前用戶為其他用戶
        return (file->mode & IXOTH);
    }
}



void touch_file(struct inode_t* working_directory, char* path) {
    if (path == NULL) {
        return;
    }
    char* filename = (char*)malloc(strlen(path));
    if (strchr(path, '/') != NULL) {
        working_directory = find_parent(working_directory, path);
        get_file_name(path, filename);
    } else {
        strcpy(filename, path);
    }
    if (working_directory == NULL) {
        printf("touch: %s: No such file or directory\n", path);
    } else if (find_file_from_parent(working_directory, filename) != NULL) {
        struct inode_t* inode = find_file_from_parent(working_directory, filename);
        inode->modified_time = time(NULL);
        inode->accessed_time = time(NULL);
    } else if (strlen(filename) > FILE_NAME_LENGTH) {
        printf("%s: Too long file name\n", path);
    } else {
        //創建目錄子項
        struct child_file_t file;
        memset(&file.filename, '\0', FILE_NAME_LENGTH);
        file.inode_number = get_free_inode();
        strcpy(file.filename, filename);
        //初始化 inode
        struct inode_t* inode = get_inode_by_num(file.inode_number);
        inode->link_count = 1;
        inode->mode = ISREG + MAX_FILE_PERMISSION - superblock.umask;
        strcpy(inode->user, current_user);
        strcpy(inode->group, current_group);
        inode->size = 0;
        inode->created_time = time(NULL);
        inode->modified_time = time(NULL);
        inode->accessed_time = time(NULL);
        //寫目錄文件
        unsigned short writing_block = (unsigned short)(working_directory->size / BLOCK_SIZE);
        size_t writing_position =working_directory->size % BLOCK_SIZE;
        //因為 inode 總數僅 64 個，填滿目錄前四塊直接索引塊需要 64 個子項，因此不可能填滿。
        if (writing_block < NADDR - 2) {
            if (writing_position == 0) {
                working_directory->data_address[writing_block] = get_free_data_block();
                if (working_directory->data_address[writing_block] == BLOCK_NUM) {
                    return_inode(inode->number);
                    return;
                }
            }
            fseek(disk, (working_directory->data_address[writing_block] + DATA_BLOCK_START) * BLOCK_SIZE + writing_position, SEEK_SET);
            fwrite(&file, sizeof(struct child_file_t), 1, disk);
            working_directory->size += sizeof(struct child_file_t);
        }
    }
}

struct inode_t* find_file_from_parent(struct inode_t* directory, char* filename) {
    if (filename == NULL || directory == NULL || (directory->mode & 07000) != ISDIR) {
        return NULL;
    } else if (strchr(filename, '/') != NULL) {
        return find_file_by_path(directory, filename);
    }

    char *data = (char *)malloc(directory->size);
    read_data(directory, data);
    struct child_file_t* directory_content = (struct child_file_t*)data;

    for (int i = 0; i < directory->size / sizeof(struct child_file_t); ++i) {
        if (strcmp(directory_content[i].filename, filename) == 0) {
            return get_inode_by_num(directory_content[i].inode_number);
        }
    }

    return NULL;
}

struct inode_t* find_file_by_path(struct inode_t* working_directory, const char* path) {
    if (path == NULL) {
        return NULL;
    }
    char* child_file = (char*)malloc(strlen(path));
    char* child_path = (char*)malloc(strlen(path));
    if (path[0] == '/') {
        working_directory = get_inode_by_num(0);
        split_relative_path(path + 1, child_file, child_path);
    } else {
        split_relative_path(path, child_file, child_path);
    }
    char* new_path = (char*)malloc(strlen(path));
    do {
        if (strlen(child_path) == 0) {
            if (strlen(child_file) == 0) {
                return working_directory;
            } else {
                return find_file_from_parent(working_directory, child_file);
            }
        }
        if (strlen(child_file) > 0) {
            working_directory = find_file_from_parent(working_directory, child_file);
        }
        strcpy(new_path, child_path);
        split_relative_path(new_path, child_file, child_path);
    } while (working_directory != NULL);
    return working_directory;
}

struct inode_t* find_parent(struct inode_t* working_directory, char* path) {
    if (path == NULL) {
        return NULL;
        //若 path 為空指針，則將 filename 置為空串
    }
    char* last_slash = strrchr(path, '/');
    if (last_slash == NULL) {
        return working_directory;
    } else if (strlen(last_slash) == strlen(path)) {
        return get_inode_by_num(0);
    } else {
        char* parent_path = malloc(strlen(path));
        memset(parent_path, '\0', strlen(path));
        strncpy(parent_path, path, strlen(path) - strlen(last_slash));
        return find_file_by_path(working_directory, parent_path);
    }
}

#endif /* functions_h */
