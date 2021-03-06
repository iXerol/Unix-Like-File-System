#include "functions.h"

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
    if (read_data == inode->size) {
        inode->accessed_time = time(NULL);
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
    if (read_data == inode->size) {
        inode->accessed_time = time(NULL);
        return;
    }

    unsigned int level_2_data_block = inode->data_address[NADDR - 1];
    unsigned int level_2_address[NADDR_BLOCK];
    fseek(disk, (level_2_data_block + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
    fread(level_2_address, sizeof(unsigned int), NADDR_BLOCK, disk);
    for (int i = 0; read_data < MAX_FILE_SIZE && read_data < inode->size; ++i) {
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
    inode->accessed_time = time(NULL);
}

void erase_data(struct inode_t* inode) {
    if (inode == NULL) {
        return;
    }
    size_t retrieveed_data = 0;

    for (unsigned short i = 0; retrieveed_data < MAX_DIRECT_FILE_SIZE && retrieveed_data < inode->size; ++i) {
        unsigned int current_data_block = inode->data_address[i];
        size_t size_to_retrieve = (inode->size - retrieveed_data >= BLOCK_SIZE) ? BLOCK_SIZE : inode->size - retrieveed_data;
        return_data_block(current_data_block);
        retrieveed_data += size_to_retrieve;
    }
    if (retrieveed_data == inode->size) {
        inode->modified_time = time(NULL);
        inode->accessed_time = time(NULL);
        inode->size = 0;
        return;
    }

    unsigned int level_1_data_block = inode->data_address[NADDR - 2];
    unsigned int level_1_address[NADDR_BLOCK];
    fseek(disk, (level_1_data_block + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
    fread(level_1_address, sizeof(unsigned int), NADDR_BLOCK, disk);
    return_data_block(level_1_data_block);
    for (unsigned short i = 0; retrieveed_data < MAX_LEVEL_1_FILE_SIZE && retrieveed_data < inode->size; ++i) {
        unsigned int current_data_block = level_1_address[i];
        size_t size_to_retrieve = inode->size - retrieveed_data >= BLOCK_SIZE ? BLOCK_SIZE : inode->size - retrieveed_data;
        return_data_block(current_data_block);
        retrieveed_data += size_to_retrieve;
    }
    if (retrieveed_data == inode->size) {
        inode->modified_time = time(NULL);
        inode->accessed_time = time(NULL);
        inode->size = 0;
        return;
    }

    unsigned int level_2_data_block = inode->data_address[NADDR - 1];
    unsigned int level_2_address[NADDR_BLOCK];
    fseek(disk, (level_2_data_block + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
    fread(level_2_address, sizeof(unsigned int), NADDR_BLOCK, disk);
    return_data_block(level_2_data_block);
    for (int i = 0; retrieveed_data < MAX_FILE_SIZE && retrieveed_data < inode->size; ++i) {
        level_1_data_block = level_2_address[i];
        fseek(disk, (level_1_data_block + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
        fread(level_1_address, sizeof(unsigned int), NADDR_BLOCK, disk);
        return_data_block(level_1_data_block);
        for (unsigned short j = 0; retrieveed_data < MAX_FILE_SIZE && retrieveed_data < inode->size; ++j) {
            unsigned int current_data_block = level_1_address[j];
            size_t size_to_retrieve = inode->size - retrieveed_data >= BLOCK_SIZE ? BLOCK_SIZE : inode->size - retrieveed_data;
            return_data_block(current_data_block);
            retrieveed_data += size_to_retrieve;
        }
    }
    inode->size = 0;
    inode->modified_time = time(NULL);
    inode->accessed_time = time(NULL);
}

void write_data(struct inode_t* inode, char* data, size_t size) {
    if (inode == NULL || data == NULL) {
        return;
    }

    for (unsigned short i = 0; inode->size < MAX_DIRECT_FILE_SIZE && inode->size < size; ++i) {
        unsigned int current_data_block = get_free_data_block();
        if (current_data_block == BLOCK_NUM) {
            return;
        }
        inode->data_address[i] = current_data_block;
        size_t size_to_write = (size - inode->size >= BLOCK_SIZE) ? BLOCK_SIZE : size - inode->size;
        fseek(disk, (current_data_block + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
        fwrite(data + inode->size, size_to_write, 1, disk);
        inode->size += size_to_write;
    }
    if (inode->size == size) {
        inode->modified_time = time(NULL);
        inode->accessed_time = time(NULL);
        return;
    }

    unsigned int level_1_data_block = get_free_data_block();
    if (level_1_data_block == BLOCK_NUM) {
        return;
    }
    inode->data_address[NADDR - 2] = level_1_data_block;
    unsigned int level_1_address[NADDR_BLOCK];
    for (unsigned short i = 0; inode->size < MAX_LEVEL_1_FILE_SIZE && inode->size < size; ++i) {
        unsigned int current_data_block = get_free_data_block();
        if (current_data_block == BLOCK_NUM) {
            return;
        }
        level_1_address[i] = current_data_block;
        size_t size_to_write = (size - inode->size >= BLOCK_SIZE) ? BLOCK_SIZE : size - inode->size;
        fseek(disk, (current_data_block + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
        fwrite(data + inode->size, size_to_write, 1, disk);
        inode->size += size_to_write;
    }
    fseek(disk, (level_1_data_block + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
    fwrite(level_1_address, sizeof(unsigned int), NADDR_BLOCK, disk);
    if (inode->size == size) {
        inode->modified_time = time(NULL);
        inode->accessed_time = time(NULL);
        return;
    }

    unsigned int level_2_data_block = get_free_data_block();
    if (level_2_data_block == BLOCK_NUM) {
        return;
    }
    inode->data_address[NADDR - 1] = level_2_data_block;
    unsigned int level_2_address[NADDR_BLOCK];
    for (int i = 0; inode->size < MAX_FILE_SIZE && inode->size < size; ++i) {
        level_1_data_block = get_free_data_block();
        if (level_1_data_block == BLOCK_NUM) {
            return;
        }
        level_2_address[i] = level_1_data_block;
        for (unsigned short j = 0; inode->size < MAX_FILE_SIZE && inode->size < size; ++j) {
            unsigned int current_data_block = get_free_data_block();
            if (current_data_block == BLOCK_NUM) {
                return;
            }
            level_1_address[j] = current_data_block;
            size_t size_to_write = (size - inode->size >= BLOCK_SIZE) ? BLOCK_SIZE : size - inode->size;
            fseek(disk, (current_data_block + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
            fwrite(data + inode->size, size_to_write, 1, disk);
            inode->size += size_to_write;
        }
        fseek(disk, (level_1_data_block + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
        fwrite(level_1_address, sizeof(unsigned int), NADDR_BLOCK, disk);
    }
    fseek(disk, (level_2_data_block + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
    fwrite(level_2_address, sizeof(unsigned int), NADDR_BLOCK, disk);
    inode->modified_time = time(NULL);
    inode->accessed_time = time(NULL);
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
    return INODE_NUM;
}

unsigned int get_free_data_block() {
    unsigned int free_data_block;
    if (superblock.stack_size > 1) {
        --superblock.stack_size;
        --superblock.num_free_block;
        free_data_block = superblock.free_block_stack[superblock.stack_size];
    } else if (superblock.stack_size == 1 && superblock.free_block_stack[0] != BLOCK_NUM){
        --superblock.stack_size;
        --superblock.num_free_block;
        free_data_block = superblock.free_block_stack[superblock.stack_size];

        fseek(disk, (superblock.free_block_stack[0] + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
        fread(superblock.free_block_stack, sizeof(unsigned int), DATA_BLOCK_STACK_SIZE, disk);
        if (superblock.free_block_stack[0] == BLOCK_NUM) {
            superblock.stack_size = superblock.num_free_block + 1;
        } else {
            superblock.stack_size = DATA_BLOCK_STACK_SIZE;
        }
    } else {
        printf("There is no enough space to create file.\n");
        return BLOCK_NUM;
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
    ++superblock.num_free_block;
    if (superblock.stack_size == DATA_BLOCK_STACK_SIZE) {
        fseek(disk, (n + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
        fwrite(superblock.free_block_stack, sizeof(unsigned int), superblock.stack_size, disk);

        superblock.free_block_stack[0] = n;
        superblock.stack_size = 1;
    } else if (superblock.stack_size < DATA_BLOCK_STACK_SIZE) {
        superblock.free_block_stack[superblock.stack_size] = n;
        ++superblock.stack_size;
    }
}


bool is_owner(struct inode_t* file) {
    if (strcmp(current_user, "root") == 0) {
        return true;
    } else if (strcmp(current_user, file->user) == 0) {
        return true;
    } else {
        return false;
    }
}

bool check_read_permission(struct inode_t* file) {
    if (file == NULL) {
        return false;
    }
    if (strcmp(current_user, "root") == 0) {
        return true;
    }
    if (strcmp(file->user, current_user) == 0) {

        return (file->mode & IRUSR);
    }
    if (strcmp(file->group, current_group) == 0) {

        return (file->mode & IRGRP);
    }

    return (file->mode & IROTH);
}

bool check_write_permission(struct inode_t* file) {
    if (file == NULL) {
        return false;
    }
    if (strcmp(current_user, "root") == 0) {
        return true;
    }
    if (strcmp(file->user, current_user) == 0) {

        return (file->mode & IWUSR);
    }
    if (strcmp(file->group, current_group) == 0) {

        return (file->mode & IWGRP);
    }

    return (file->mode & IWOTH);
}

bool check_execute_permission(struct inode_t* file) {
    if (file == NULL) {
        return false;
    }
    if (strcmp(current_user, "root") == 0) {
        return true;
    }
    if (strcmp(file->user, current_user) == 0) {

        return (file->mode & IXUSR);
    }
    if (strcmp(file->group, current_group) == 0) {

        return (file->mode & IXGRP);
    }

    return (file->mode & IXOTH);
}

bool is_descendant_directory(struct inode_t* ancestor, struct inode_t* descendant) {
    if ((ancestor->mode & 07000) != ISDIR || (descendant->mode & 07000) != ISDIR) {
        return false;
    }
    struct inode_t* root = get_inode_by_num(0);
    if (ancestor == root) {
        return true;
    }

    while (descendant != ancestor && descendant != root) {
        descendant = find_file_from_parent(descendant, "..");
    }

    return descendant == ancestor;
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
    if (strcmp(path, "~") == 0) {
        char user_directory[USER_NAME_LENGTH + 8] = "/home/";
        strcat(user_directory, current_user);
        return find_file_by_path(working_directory, user_directory);
    }
    if (path[0] == '/') {
        working_directory = get_inode_by_num(0);
        split_relative_path(path + 1, child_file, child_path);
    } else if (path[0] == '~' && path[1] == '/') {
        char user_directory[USER_NAME_LENGTH + 8] = "/home/";
        strcat(user_directory, current_user);
        working_directory = find_file_by_path(working_directory, user_directory);
        split_relative_path(path + 2, child_file, child_path);
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
