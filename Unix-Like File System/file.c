#include "file.h"

void new_volume() {
    disk = fopen(FILENAME, "wb+");
    if (disk == NULL) {
        printf("failed to initialize file system.\n");
    } else {
        initialize_superblock();
        initialize_inodes();
        initialize_data_block();

        fseek(disk, BLOCK_SIZE, SEEK_SET);
        fwrite(&superblock, sizeof(struct superblock_t), 1, disk);

        for (int i = 0; i < INODE_NUM; ++i) {
            write_inode(i);
        }

        strcpy(current_user, "root");
        strcpy(current_group, "wheel");

        create_root();

        struct inode_t* root = get_inode_by_num(0);

        current_working_inode = get_inode_by_num(0);
        strcpy(current_working_directory, "/");

        create_directory(root, "/etc");
        create_directory(root, "/home");
        touch_file("etc/passwd");
        create_user("root", "root", "wheel");
        create_user("ixerol", "123456", "staff");
        create_user("zero", "123456", "staff");
        create_user("test", "test", "test");
        save();
    }
}

void mount_volume() {
    disk = fopen(FILENAME, "rb+");
    if (disk == NULL) {
        printf("failed to mount file system.\n");
    } else {
        fseek(disk, BLOCK_SIZE, SEEK_SET);
        fread(&superblock, sizeof(struct superblock_t), 1, disk);

        for (unsigned int i = 0; i < INODE_NUM; ++i) {
            fseek(disk, INODE_BLOCK_START * BLOCK_SIZE + INODE_SIZE * i, SEEK_SET);
            fread(inodes + i, sizeof(struct inode_t), 1, disk);
        }

        current_working_inode = get_inode_by_num(0);
        strcpy(current_working_directory, "/");
    }
}

void save() {
    fseek(disk, BLOCK_SIZE, SEEK_SET);
    fwrite(&superblock, sizeof(superblock), 1, disk);

    for (int i = 0; i < INODE_NUM; ++i) {
        write_inode(i);
    }
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
    unsigned int free_block_stack[DATA_BLOCK_STACK_SIZE];
    unsigned int num_data_block = BLOCK_NUM - DATA_BLOCK_START;
    unsigned int i;
    superblock.stack_size = DATA_BLOCK_STACK_SIZE;
    for (i = 0; i < DATA_BLOCK_STACK_SIZE; ++i) {
        superblock.free_block_stack[i] = i;
    }
    fseek(disk, (superblock.free_block_stack[0] + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
    while (i < num_data_block) {
        stack_size = (num_data_block - i < DATA_BLOCK_STACK_SIZE) ? num_data_block - i : DATA_BLOCK_STACK_SIZE;
        for (int j = 0; j < stack_size; ++i, ++j) {
            free_block_stack[j] = i;
        }
        if (stack_size < DATA_BLOCK_STACK_SIZE) {
            free_block_stack[stack_size] = free_block_stack[0];
            free_block_stack[0] = BLOCK_NUM;
            ++stack_size;
        }
        fwrite(free_block_stack, sizeof(unsigned int), stack_size, disk);
        if (free_block_stack[0] != BLOCK_NUM) {
            fseek(disk, (free_block_stack[0] + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
        }
    }
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

void write_inode(int n) {
    fseek(disk, INODE_BLOCK_START * BLOCK_SIZE + n * INODE_SIZE, SEEK_SET);
    fwrite(inodes + n, sizeof(struct inode_t), 1, disk);
}
