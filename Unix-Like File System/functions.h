#ifndef functions_h
#define functions_h

#include "variables.h"
#include "path.h"
#include <stdlib.h>

void show(void);

void initialize_superblock(void);

void initialize_inodes(void);

void initialize_data_block(void);

void write_inode(int n);

void create_root(void);

void read_data(struct inode_t* inode, char* data);

struct inode_t* get_inode_by_num(unsigned int n);

unsigned int get_free_inode(void);

unsigned int get_free_data_block(void);

void link_file(struct inode_t* working_directory, char* target_file_path, char* source_file_path);

struct inode_t* find_file_from_parent(struct inode_t* directory, char *filename);

void create_directory(struct inode_t* working_directory, char* directory_path);

struct inode_t* find_file_by_path(struct inode_t* current_inode, char* path);

struct inode_t* find_parent(struct inode_t* working_directory, char* path);

void show() {
    printf("free data block stack size: %zu\n", superblock.stack_size);
    printf("free data block stack:");
    for (int i = 0; i < superblock.stack_size; ++i) {
        printf(" %d", superblock.free_block_stack[i]);
    }
    printf("\n\nfree inode number: %u\ninodes:", superblock.num_free_inode);
    for (int i = 0; i < INODE_NUM; ++i) {
        printf(" %d", (superblock.free_inodes & ((uint64_t)(1) << i)) != 0 ? i : -1);
    }
    printf("\n");
}

void initialize_superblock() {
    superblock.num_free_inode = INODE_NUM;

    superblock.num_free_block = BLOCK_NUM - DATA_BLOCK_START;

    superblock.umask = DEFAULT_UMASK;

    superblock.free_inodes = UINT64_MAX;
}

void initialize_inodes() {
    memset(inodes, 0, sizeof(struct inode_t) * INODE_NUM);
    for (int i = 0; i < INODE_NUM; ++i) {
        inodes[i].number = i;
    }
}

void initialize_data_block() {
    size_t stack_size;
    unsigned int free_block_stack[100];
    unsigned int num_data_block = BLOCK_NUM - DATA_BLOCK_START;
    unsigned int i;
    superblock.stack_size = 100;
    for (i = 0; i < 100; ++i) {
        superblock.free_block_stack[i] = i;
    }
    fseek(disk, (superblock.free_block_stack[0] + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
    while (i < num_data_block) {
        stack_size = (num_data_block - i < 100) ? num_data_block - i : 100;
        for (int j = 0; j < stack_size; ++i, ++j) {
            free_block_stack[j] = i;
        }
        if (stack_size < 100) {
            free_block_stack[stack_size] = free_block_stack[0];
            free_block_stack[0] = BLOCK_NUM;
            ++stack_size;
        }
        fwrite(&stack_size, sizeof(size_t), 1, disk);
        fwrite(free_block_stack, sizeof(unsigned int), stack_size, disk);
        if (free_block_stack[0] != BLOCK_NUM) {
            fseek(disk, (free_block_stack[0] + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
        }
    }
}

void write_inode(int n) {
    fseek(disk, INODE_BLOCK_START * BLOCK_SIZE + n * INODE_SIZE, SEEK_SET);
    fwrite(inodes + n, sizeof(struct inode_t), 1, disk);
}

void create_root() {
    //inodes[0] 對應根目錄
    inodes[0].link_count = 1;
    strcpy(inodes[0].user, current_user);
    strcpy(inodes[0].group, current_group);
    inodes[0].mode = ISDIR + MAX_DIRECTORY_PERMISSION - superblock.umask;
    superblock.free_inodes = superblock.free_inodes ^ (1 << get_free_inode());

    link_file(get_inode_by_num(0), ".", "/");
    link_file(get_inode_by_num(0), "..", "/");
}

void read_data(struct inode_t* inode, char* data) {
    //读取pInode指向的inode块数据
    size_t read_data = 0;

    for (unsigned short i = 0; read_data < MAX_DIRECT_FILE_SIZE && read_data < inode->size; ++i) {
        unsigned int current_data_block = inode->data_address[i];
        size_t size_to_read = inode->size - read_data >= BLOCK_SIZE ? BLOCK_SIZE : inode->size - read_data;
        fseek(disk, (current_data_block + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
        fread(data + read_data, size_to_read, 1, disk);
        read_data += size_to_read;
        if (read_data == inode->size) {
            return;
        }
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
        if (read_data == inode->size) {
            return;
        }
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
            if (read_data == inode->size) {
                return;
            }
        }
    }
}

struct inode_t* get_inode_by_num(unsigned int n) {
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
    } else if (superblock.free_block_stack[0] != BLOCK_NUM){
        fseek(disk, (superblock.free_block_stack[0] + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
        fread(&superblock.stack_size, sizeof(size_t), 1, disk);
        fread(superblock.free_block_stack, sizeof(unsigned int), superblock.stack_size, disk);

        --superblock.stack_size;
        free_data_block = superblock.free_block_stack[superblock.stack_size];
    } else {
        printf("There is no enough space to create file.\n");
        return BLOCK_NUM;   //磁盤容量不足
    }
    return free_data_block;
}


void link_file(struct inode_t* working_directory, char* target_file_path, char* source_file_path) {
    if (target_file_path == NULL || source_file_path == NULL) {
        return;
    }
    struct inode_t* source_file = find_file_by_path(working_directory, source_file_path);
    struct inode_t* target_parent = find_parent(working_directory, target_file_path);
    char* target_file_name = (char*)malloc(strlen(target_file_path));
    get_file_name(target_file_path, target_file_name);
    if (source_file == NULL) {
        printf("ln: %s: No such file or directory\n", source_file_path);
        return;
    } else if (target_parent == NULL) {
        printf("ln: %s: No such file or directory\n", target_file_path);
        return;
    } else if (find_file_from_parent(target_parent, target_file_name) != NULL) {
        printf("ln: %s: File exists\n", target_file_path);
        return;
    }

    //創建目錄子項
    struct child_file_t new_link_file;
    strcpy(new_link_file.filename, target_file_name);
    new_link_file.inode_number = source_file->number;
    //寫目錄文件
    unsigned short writing_block = (unsigned short)(target_parent->size / BLOCK_SIZE);
    size_t writing_position =target_parent->size % BLOCK_SIZE;
    //因為 inode 總數僅 64 個，填滿目錄前四塊直接索引塊需要 64 個子項，因此不可能填滿。
    if (writing_block < NADDR - 2) {
        if (writing_position == 0) {
            target_parent->data_address[writing_block] = get_free_data_block();
            if (target_parent->data_address[writing_block] == BLOCK_NUM) {
                return;
            }
        }
        fseek(disk, (target_parent->data_address[writing_block] + DATA_BLOCK_START) * BLOCK_SIZE + writing_position, SEEK_SET);
        fwrite(&new_link_file, sizeof(struct child_file_t), 1, disk);
        target_parent->size += sizeof(struct child_file_t);
        ++target_parent->link_count;
    }
}

void create_directory(struct inode_t* working_directory, char* directory_path) {
    if (working_directory == NULL || directory_path == NULL) {
        return;
    }
    char* directory_name = (char*)malloc(strlen(directory_path));
    if (strchr(directory_path, '/') != NULL) {
        working_directory = find_parent(working_directory, directory_path);
        get_file_name(directory_path, directory_name);
    } else {
        strcpy(directory_name, directory_path);
    }
    if (working_directory == NULL) {
        printf("mkdir: %s: No such file or directory\n", directory_path);

    } else if (strlen(directory_name) > FILE_NAME_LENGTH) {
        printf("%s: Too long directory name\n", directory_path);
    } else {
        //創建目錄子項
        struct child_file_t directory;
        directory.inode_number = get_free_inode();
        strcpy(directory.filename, directory_name);
        //初始化 inode
        struct inode_t* inode = get_inode_by_num(directory.inode_number);
        inode->link_count = 1;
        inode->mode = ISDIR + MAX_DIRECTORY_PERMISSION - superblock.umask;
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
                    return;
                }
            }
            fseek(disk, (working_directory->data_address[writing_block] + DATA_BLOCK_START) * BLOCK_SIZE + writing_position, SEEK_SET);
            fwrite(&directory, sizeof(struct child_file_t), 1, disk);
            working_directory->size += sizeof(struct child_file_t);
            //創建 . .. 目錄
            char* dot_path = (char*)malloc(strlen(directory_name) + 4);
            strcpy(dot_path, directory_name);
            strcat(dot_path, "/.");
            link_file(working_directory, dot_path, directory_name);
            strcat(dot_path, ".");
            link_file(working_directory, dot_path, ".");
        }
    }
}

struct inode_t *find_file_from_parent(struct inode_t* directory, char *filename) {
    if (filename == NULL || directory == NULL || directory->mode / 01000 != ISDIR / 01000) {
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

struct inode_t* find_file_by_path(struct inode_t* working_directory, char* path) {
    if (path == NULL) {
        return NULL;
    }
    char child_file[FILE_NAME_LENGTH] = "";
    char* child_path = (char*)malloc(strlen(path) + 1);
    if (path[0] == '/') {
        working_directory = get_inode_by_num(0);
        split_relative_path(path + 1, child_file, child_path);
    } else {
        split_relative_path(path, child_file, child_path);
    }
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
        strcpy(path, child_path);
        split_relative_path(path, child_file, child_path);
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
        strcpy(parent_path, "");
        strncpy(parent_path, path, strlen(path) - strlen(last_slash));
        return find_file_by_path(working_directory, parent_path);
    }
}

#endif /* functions_h */
