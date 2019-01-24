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

void touch_file(char* path);

void resize_text_file(struct inode_t* inode, size_t new_size);

void remove_regular_file(char* path);

void cat(char* path);

void show_umask(void);

void change_umask(unsigned short new_umask);


void show() {
    printf("free data block: %u\n", superblock.num_free_block);
    printf("\nfree data block stack size: %zu\n", superblock.stack_size);
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

void create_user(char* username, char* password, char* group) {
    if (strcmp(current_user, "root") != 0) {
        printf("useradd: Permission denied\n");
    }
    if (username == NULL || password == NULL || group == NULL) {
        return;
    }
    if (strlen(username) <= USER_NAME_LENGTH && strlen(password) <= USER_PASSWORD_LENGTH && strlen(group) <= GROUP_NAME_LENGTH) {
        struct inode_t* passwd = find_file_by_path(current_working_inode, "/etc/passwd");
        if (passwd == NULL) {
            passwd = find_file_by_path(current_working_inode, "/etc");
            if (passwd == NULL) {
                create_directory(current_working_inode, "/etc");
            }
            touch_file("/etc/passwd");
            passwd = find_file_by_path(current_working_inode, "/etc/passwd");
        }
        size_t user_size = passwd->size;
        char* user_data = malloc(user_size + USER_DATA_LENGTH);
        read_data(passwd, user_data);
        char tmp_username[USER_NAME_LENGTH];

        for (size_t i = 0; i < user_size / USER_DATA_LENGTH; ++i) {
            memset(tmp_username, 0, USER_NAME_LENGTH);
            strncpy(tmp_username, user_data + i * USER_DATA_LENGTH, USER_NAME_LENGTH);
            if (strcmp(username, tmp_username) == 0) {
                printf("User already exists.\n");
                return;
            }
        }

        strcpy(user_data + user_size, username);
        strcpy(user_data + user_size + USER_NAME_LENGTH, password);
        strcpy(user_data + user_size + USER_NAME_LENGTH + USER_PASSWORD_LENGTH, group);
        erase_data(passwd);
        write_data(passwd, user_data, user_size + USER_DATA_LENGTH);

        
        strcpy(current_user, username);
        strcpy(current_group, group);
        char user_directory[USER_NAME_LENGTH + 8] = "/home/";
        strcat(user_directory, username);
        create_directory(current_working_inode, user_directory);

        strcpy(current_user, "root");
        strcpy(current_group, "wheel");
    }
}

void change_password(char* old_password, char* new_password) {
    if (new_password == NULL) {
        return;
    }
    if (strlen(new_password) > USER_PASSWORD_LENGTH) {
        printf("New password is too long\n");
    } else {
        struct inode_t* passwd = find_file_by_path(current_working_inode, "/etc/passwd");
        size_t passwd_size = passwd->size;

        char* user_data = malloc(passwd->size);
        read_data(passwd, user_data);
        char tmp_username[USER_NAME_LENGTH];
        char tmp_password[USER_PASSWORD_LENGTH];

        for (size_t i = 0; i < passwd->size / USER_DATA_LENGTH; ++i) {
            memset(tmp_username, 0, USER_NAME_LENGTH);
            memset(tmp_password, 0, USER_PASSWORD_LENGTH);
            strncpy(tmp_username, user_data + i * USER_DATA_LENGTH, USER_NAME_LENGTH);
            strncpy(tmp_password, user_data + i * USER_DATA_LENGTH + USER_NAME_LENGTH, USER_PASSWORD_LENGTH);
            if (strcmp(current_user, tmp_username) == 0) {
                if (strcmp(tmp_password, old_password) != 0) {
                    printf("passwd: authentication token failure\n");
                    return;
                }
                strcpy(user_data + i * USER_DATA_LENGTH + USER_NAME_LENGTH, new_password);
                break;
            }
        }

        erase_data(passwd);
        write_data(passwd, user_data, passwd_size);
    }
}

void change_mode(char* path, unsigned short privilege) {
    if (path == NULL) {
        return;
    }
    if (privilege > 0777) {
        printf("chmod: Invalid file mode: %03o\n", privilege);
    } else {
        struct inode_t* file = find_file_by_path(current_working_inode, path);
        if (file == NULL) {
            printf("chmod: %o: No such file or directory\n", privilege);
        } else if (!is_owner(file)){
            printf("chmod: Unable to change file mode on %s: Operation not permitted\n", path);
        } else {
            file->mode = (file->mode & 07000) + privilege;
            file->accessed_time = time(NULL);
        }
    }
}

void change_owner(char* path, char* user) {
    if (path == NULL || user == NULL) {
        return;
    }
    struct inode_t* inode = find_file_by_path(current_working_inode, path);
    if (inode == NULL) {
        printf("chown: %s: No such file or directory\n", path);
        return;
    }
    if (!is_owner(inode)) {
        printf("chown: %s: Operation not permitted\n", path);
    }

    struct inode_t* passwd = find_file_by_path(current_working_inode, "/etc/passwd");
    char* user_data = malloc(passwd->size);
    read_data(passwd, user_data);
    char tmp_username[USER_NAME_LENGTH];

    for (size_t i = 0; i < passwd->size / USER_DATA_LENGTH; ++i) {
        memset(tmp_username, 0, USER_NAME_LENGTH);
        strncpy(tmp_username, user_data + i * USER_DATA_LENGTH, USER_NAME_LENGTH);
        if (strcmp(user, tmp_username) == 0) {
            strcpy(inode->user, user);
            return;
        }
    }
    printf("chown: %s: illegal user name\n", user);
}

void change_group(char* path, char* group) {
    if (path == NULL || group == NULL) {
        return;
    }
    struct inode_t* inode = find_file_by_path(current_working_inode, path);
    if (inode == NULL) {
        printf("chgrp: %s: No such file or directory\n", path);
        return;
    }
    if (!is_owner(inode)) {
        printf("chgrp: %s: Operation not permitted\n", path);
    }

    struct inode_t* passwd = find_file_by_path(current_working_inode, "/etc/passwd");
    char* user_data = malloc(passwd->size);
    read_data(passwd, user_data);
    char tmp_group[GROUP_NAME_LENGTH];

    for (size_t i = 0; i < passwd->size / USER_DATA_LENGTH; ++i) {
        memset(tmp_group, 0, GROUP_NAME_LENGTH);
        strncpy(tmp_group, user_data + i * USER_DATA_LENGTH + USER_NAME_LENGTH + USER_PASSWORD_LENGTH, GROUP_NAME_LENGTH);
        if (strcmp(group, tmp_group) == 0) {
            strcpy(inode->group, group);
            return;
        }
    }
    printf("chgrp: %s: illegal group name\n", group);
}

void present_working_directory() {
    if (current_working_inode->number == 0) {
        printf("/\n");
        return;
    }

    struct inode_t* working_directory = current_working_inode;
    struct inode_t* worked_directory = current_working_inode;
    char working_directory_string[(FILE_NAME_LENGTH + 1) * INODE_NUM];
    memset(working_directory_string, '\0', (FILE_NAME_LENGTH + 1) * INODE_NUM);
    char worked_directory_string[(FILE_NAME_LENGTH + 1) * INODE_NUM];
    memset(worked_directory_string, '\0', (FILE_NAME_LENGTH + 1) * INODE_NUM);
    char* data = (char*)malloc(sizeof(struct child_file_t) * INODE_NUM);

    read_data(working_directory, data);
    struct child_file_t* directory_content = (struct child_file_t*)data;
    for (int i = 0; i < working_directory->size / sizeof(struct child_file_t); ++i) {
        if (strcmp(directory_content[i].filename, "..") == 0) {
            worked_directory = working_directory;
            working_directory = get_inode_by_num(directory_content[i].inode_number);
            break;
        }
    }
    while (working_directory->number != 0) {
        read_data(working_directory, data);
        for (int i = 0; i < working_directory->size / sizeof(struct child_file_t); ++i) {
            if (directory_content[i].inode_number == worked_directory->number) {
                strcpy(working_directory_string, "/");
                strcat(working_directory_string, directory_content[i].filename);
                strcat(working_directory_string, worked_directory_string);
                strcpy(worked_directory_string, working_directory_string);
                break;
            }
        }
        directory_content = (struct child_file_t*)data;
        for (int i = 0; i < working_directory->size / sizeof(struct child_file_t); ++i) {
            if (strcmp(directory_content[i].filename, "..") == 0) {
                worked_directory = working_directory;
                working_directory = get_inode_by_num(directory_content[i].inode_number);
                break;
            }
        }
    }
    read_data(working_directory, data);
    for (int i = 0; i < working_directory->size / sizeof(struct child_file_t); ++i) {
        if (directory_content[i].inode_number == worked_directory->number) {
            strcpy(working_directory_string, "/");
            strcat(working_directory_string, directory_content[i].filename);
            strcat(working_directory_string, worked_directory_string);
            strcpy(worked_directory_string, working_directory_string);
            
        }
    }

    printf("%s\n", working_directory_string);
}

void list(char* path) {
    struct inode_t* directory = current_working_inode;
    if (path != NULL && strcmp(path, "") != 0) {
        directory = find_file_by_path(current_working_inode, path);
    }
    if (directory == NULL) {
        printf("ls: %s: No such file or directory\n", path);
    } else if (!check_read_permission(directory)) {
        printf("ls: %s: Permission denied\n", path);
    } else if ((directory->mode & 07000) == ISDIR) {
        char *data = (char *)malloc(directory->size);
        read_data(directory, data);
        struct child_file_t* directory_content = (struct child_file_t*)data;

        

        for (int i = 0; i < directory->size / sizeof(struct child_file_t); ++i) {
            struct inode_t* inode = get_inode_by_num(directory_content[i].inode_number);
            switch (inode->mode & 07000) {
                case ISREG:
                    putchar('-');
                    break;
                case ISDIR:
                    putchar('d');
                    break;
                case ISCHR:
                    putchar('c');
                    break;
                case ISBLK:
                    putchar('b');
                    break;
                case ISLNK:
                    putchar('l');
                    break;
                case ISFIFO:
                    putchar('p');
                    break;
                case ISSOCK:
                    putchar('s');
                    break;
                default:
                    break;
            }
            printf("%c%c%c%c%c%c%c%c%c ", (inode->mode & IRUSR) != 0 ? 'r' : '-',
                   (inode->mode & IWUSR) != 0 ? 'w' : '-',
                   (inode->mode & IXUSR) != 0 ? 'x' : '-',
                   (inode->mode & IRGRP) != 0 ? 'r' : '-',
                   (inode->mode & IWGRP) != 0 ? 'w' : '-',
                   (inode->mode & IXGRP) != 0 ? 'x' : '-',
                   (inode->mode & IROTH) != 0 ? 'r' : '-',
                   (inode->mode & IWOTH) != 0 ? 'w' : '-',
                   (inode->mode & IXOTH) != 0 ? 'x' : '-');

            printf("%3d ", inode->link_count);
            printf("%32s%32s", inode->user, inode->group);
            printf("%8zu", inode->size);

            char time[26];
            strcpy(time,  asctime(localtime(&inode->modified_time)));
            time[24] = ' ';
            printf(" %s",time);
            printf("%s\n", directory_content[i].filename);
        }
    } else {
        struct inode_t* inode = directory;
        switch (inode->mode & 07000) {
            case ISREG:
                putchar('-');
                break;
            case ISDIR:
                putchar('d');
                break;
            case ISCHR:
                putchar('c');
                break;
            case ISBLK:
                putchar('b');
                break;
            case ISLNK:
                putchar('l');
                break;
            case ISFIFO:
                putchar('p');
                break;
            case ISSOCK:
                putchar('s');
                break;
            default:
                break;
        }
        printf("%c%c%c%c%c%c%c%c%c ", (inode->mode & IRUSR) != 0 ? 'r' : '-',
               (inode->mode & IWUSR) != 0 ? 'w' : '-',
               (inode->mode & IXUSR) != 0 ? 'x' : '-',
               (inode->mode & IRGRP) != 0 ? 'r' : '-',
               (inode->mode & IWGRP) != 0 ? 'w' : '-',
               (inode->mode & IXGRP) != 0 ? 'x' : '-',
               (inode->mode & IROTH) != 0 ? 'r' : '-',
               (inode->mode & IWOTH) != 0 ? 'w' : '-',
               (inode->mode & IXOTH) != 0 ? 'x' : '-');

        printf("%3d ", inode->link_count);
        printf("%32s%32s", inode->user, inode->group);
        printf("%8zu", inode->size);

        char time[26];
        strcpy(time,  asctime(localtime(&inode->modified_time)));
        time[24] = ' ';
        printf(" %s",time);
        printf("%s\n", path);
    }
}

void status(struct inode_t* directory, char* path) {
    if (path == NULL) {
        path = (char*)malloc(10);
        strcpy(path, "/");
    }
    struct inode_t* inode = find_file_by_path(directory, path);
    if (inode == NULL) {
        printf("stat: %s: stat: No such file or directory\n", path);
    } else {
        printf("File: %s\n", path);
        printf("Size: %zu\n", inode->size);
        printf("Permission: ");
        switch (inode->mode & 07000) {
            case ISREG:
                putchar('-');
                break;
            case ISDIR:
                putchar('d');
                break;
            case ISCHR:
                putchar('c');
                break;
            case ISBLK:
                putchar('b');
                break;
            case ISLNK:
                putchar('l');
                break;
            case ISFIFO:
                putchar('p');
                break;
            case ISSOCK:
                putchar('s');
                break;
            default:
                break;
        }
        printf("%c%c%c%c%c%c%c%c%c\n", (inode->mode & IRUSR) != 0 ? 'r' : '-',
               (inode->mode & IWUSR) != 0 ? 'w' : '-',
               (inode->mode & IXUSR) != 0 ? 'x' : '-',
               (inode->mode & IRGRP) != 0 ? 'r' : '-',
               (inode->mode & IWGRP) != 0 ? 'w' : '-',
               (inode->mode & IXGRP) != 0 ? 'x' : '-',
               (inode->mode & IROTH) != 0 ? 'r' : '-',
               (inode->mode & IWOTH) != 0 ? 'w' : '-',
               (inode->mode & IXOTH) != 0 ? 'x' : '-');
        printf("User: %s\n", inode->user);
        printf("Group: %s\n", inode->group);
        printf("Created Time: %s", asctime(localtime(&inode->created_time)));
        printf("Last Modified Time: %s", asctime(localtime(&inode->modified_time)));
        printf("Last Accessed Time: %s", asctime(localtime(&inode->accessed_time)));
        printf("inode number: %u\n", inode->number);
        printf("Link Count: %u\n", inode->link_count);
    }
}

void cd(char* path) {
    if (path == NULL) {
        return;
    }
    struct inode_t* inode = find_file_by_path(current_working_inode, path);
    if (inode == NULL) {
        printf("cd: %s: No such file or directory\n", path);
    } else if ((inode->mode & 07000) != ISDIR) {
        printf("cd: %s: Not a directory\n", path);
    } else {
        current_working_inode = inode;

        if (inode->number == 0) {
            strcpy(current_working_directory, "/");
        } else {
            struct inode_t* parent = find_file_from_parent(inode, "..");

            char *data = (char *)malloc(parent->size);
            read_data(parent, data);
            struct child_file_t* directory_content = (struct child_file_t*)data;

            for (int i = 0; i < parent->size / sizeof(struct child_file_t); ++i) {
                if (directory_content[i].inode_number == inode->number) {
                    strcpy(current_working_directory, directory_content[i].filename);
                    break;
                }
            }
        }
    }
}

void copy_file(char* source_file_path, char* target_file_path) {
    if (target_file_path == NULL || source_file_path == NULL) {
        return;
    }
    struct inode_t* source_file = find_file_by_path(current_working_inode, source_file_path);
    struct inode_t* target_parent = find_parent(current_working_inode, target_file_path);
    struct inode_t* target_file;
    char* target_file_name = (char*)malloc(strlen(target_file_path));
    get_file_name(target_file_path, target_file_name);
    if (source_file == NULL) {
        printf("cp: %s: No such file or directory\n", source_file_path);
        return;
    } else if ((source_file->mode & 07000) == ISDIR) {
        printf("cp: %s is a directory (not copied).\n", source_file_path);
        return;
    } else if (target_parent == NULL) {
        printf("cp: %s: No such file or directory\n", target_file_path);
        return;
    } else {
        target_file = find_file_from_parent(target_parent, target_file_name);
    }

    if (target_file != NULL) {
        if ((target_file->mode & 07000) != ISDIR) {
            printf("cp: %s: File exists\n", target_file_path);
            return;
        } else {
            target_parent = target_file;
            get_file_name(source_file_path, target_file_name);
            if (find_file_from_parent(target_parent, target_file_name) != NULL) {
                printf("cp: %s/%s: File exists\n", target_file_path, target_file_name);
                return;
            }
        }
    }

    if (!is_legal_file_name(target_file_name)) {
        printf("cp: %s: Illegal filename\n", target_file_name);
        return;
    } else if (!check_read_permission(source_file)) {
        printf("cp: %s: Permission denied\n", source_file_path);
    }

    
    struct child_file_t new_file;
    memset(&new_file, '\0', sizeof(struct child_file_t));
    strcpy(new_file.filename, target_file_name);
    new_file.inode_number = get_free_inode();
    
    unsigned short writing_block = (unsigned short)(target_parent->size / BLOCK_SIZE);
    size_t writing_position =target_parent->size % BLOCK_SIZE;
    
    if (writing_block < NADDR - 2) {
        if (writing_position == 0) {
            target_parent->data_address[writing_block] = get_free_data_block();
            if (target_parent->data_address[writing_block] == BLOCK_NUM) {
                return;
            }
        }
        fseek(disk, (target_parent->data_address[writing_block] + DATA_BLOCK_START) * BLOCK_SIZE + writing_position, SEEK_SET);
        fwrite(&new_file, sizeof(struct child_file_t), 1, disk);
        target_parent->size += sizeof(struct child_file_t);
        ++source_file->link_count;
        source_file->accessed_time = time(NULL);
    }

    
    struct inode_t* new_inode = get_inode_by_num(new_file.inode_number);
    new_inode->link_count = 1;
    new_inode->mode = source_file->mode;
    strcpy(new_inode->user, current_user);
    strcpy(new_inode->group, current_group);
    new_inode->size = 0;
    new_inode->created_time = time(NULL);
    new_inode->modified_time = time(NULL);
    new_inode->accessed_time = time(NULL);

    
    char* data = (char*)malloc(source_file->size);
    read_data(source_file, data);
    write_data(new_inode, data, source_file->size);
}

void move_file(char* source_file_path, char* target_file_path) {
    if (target_file_path == NULL || source_file_path == NULL) {
        return;
    }
    struct inode_t* source_parent = find_parent(current_working_inode, source_file_path);
    struct inode_t* source_file;
    char* source_file_name = (char*)malloc(strlen(source_file_path));
    get_file_name(source_file_path, source_file_name);
    if (source_parent == NULL) {
        printf("mv: %s: No such file or directory\n", source_file_path);
        return;
    }
    source_file = find_file_from_parent(source_parent, source_file_name);

    
    if (strcmp(source_file_name, ".") == 0 || strcmp(source_file_name, "..") == 0) {
        printf("mv: rename %s to %s: Invalid argument\n", source_file_path, target_file_path);
    }

    struct inode_t* target_parent = find_parent(current_working_inode, target_file_path);
    struct inode_t* target_file;
    char* target_file_name = (char*)malloc(strlen(target_file_path));
    get_file_name(target_file_path, target_file_name);
    if (source_file == NULL) {
        printf("mv: %s: No such file or directory\n", source_file_path);
        return;
    } else if (target_parent == NULL) {
        printf("mv: %s: No such file or directory\n", target_file_path);
        return;
    } else {
        target_file = find_file_from_parent(target_parent, target_file_name);
    }

    if (target_file != NULL) {
        if ((target_file->mode & 07000) != ISDIR) {
            printf("mv: %s: File exists\n", target_file_path);
            return;
        } else {
            target_parent = target_file;
            get_file_name(source_file_path, target_file_name);
            if (find_file_from_parent(target_parent, target_file_name) != NULL) {
                printf("mv: %s/%s: File exists\n", target_file_path, target_file_name);
                return;
            }
        }
    }

    if (!is_legal_file_name(target_file_name)) {
        printf("mv: %s: Illegal filename\n", target_file_name);
        return;
    } else if (!check_write_permission(source_file)) {
        printf("mv: %s: Permission denied\n", source_file_path);
    }

    
    if (is_descendant_directory(source_file, target_parent)) {
        printf("mv: rename %s to %s: Invalid argument\n", source_file_path, target_file_path);
        return;
    }

    
    size_t parent_size = source_parent->size;
    char *source_parent_directory_data = (char *)malloc(parent_size);
    read_data(source_parent, source_parent_directory_data);
    struct child_file_t* directory_content = (struct child_file_t*)source_parent_directory_data;
    for (unsigned i = 0; i < parent_size / sizeof(struct child_file_t); ++i) {
        if (strcmp(directory_content[i].filename, source_file_name) == 0) {
            directory_content[i] = directory_content[parent_size / sizeof(struct child_file_t) - 1];
            erase_data(source_parent);
            write_data(source_parent, source_parent_directory_data, parent_size - sizeof(struct child_file_t));
        }
    }

    
    struct child_file_t new_file;
    memset(&new_file, '\0', sizeof(struct child_file_t));
    strcpy(new_file.filename, target_file_name);
    new_file.inode_number = source_file->number;
    
    unsigned short writing_block = (unsigned short)(target_parent->size / BLOCK_SIZE);
    size_t writing_position =target_parent->size % BLOCK_SIZE;
    
    if (writing_block < NADDR - 2) {
        if (writing_position == 0) {
            target_parent->data_address[writing_block] = get_free_data_block();
            if (target_parent->data_address[writing_block] == BLOCK_NUM) {
                return;
            }
        }
        fseek(disk, (target_parent->data_address[writing_block] + DATA_BLOCK_START) * BLOCK_SIZE + writing_position, SEEK_SET);
        fwrite(&new_file, sizeof(struct child_file_t), 1, disk);
        target_parent->size += sizeof(struct child_file_t);
        ++source_file->link_count;
        source_file->accessed_time = time(NULL);
    }

    
    if ((source_file->mode & 07000) == ISDIR) {
        char *data = (char *)malloc(source_file->size);
        read_data(source_file, data);
        struct child_file_t* directory_content = (struct child_file_t*)data;

        for (int i = 0; i < source_file->size / sizeof(struct child_file_t); ++i) {
            if (strcmp(directory_content[i].filename, "..") == 0) {
                directory_content[i].inode_number = target_parent->number;
                size_t size = source_file->size;
                erase_data(source_file);
                write_data(source_file, data, size);

                --source_parent->link_count;
                ++target_parent->link_count;
            }
        }
    }
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
    } else if (!is_legal_file_name(target_file_name)) {
        printf("ln: %s: Illegal filename\n", target_file_name);
        return;
    }

    
    struct child_file_t new_link_file;
    memset(&new_link_file, '\0', sizeof(struct child_file_t));
    strcpy(new_link_file.filename, target_file_name);
    new_link_file.inode_number = source_file->number;
    
    unsigned short writing_block = (unsigned short)(target_parent->size / BLOCK_SIZE);
    size_t writing_position =target_parent->size % BLOCK_SIZE;
    
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
        ++source_file->link_count;
        source_file->accessed_time = time(NULL);
    }
}

void create_directory(struct inode_t* working_directory, char* directory_path) {
    if (directory_path == NULL) {
        return;
    }
    struct inode_t* parent_directory;
    char* directory_name = (char*)malloc(strlen(directory_path));
    if (strchr(directory_path, '/') != NULL) {
        parent_directory = find_parent(current_working_inode, directory_path);
        get_file_name(directory_path, directory_name);
    } else {
        parent_directory = current_working_inode;
        strcpy(directory_name, directory_path);
    }
    if (parent_directory == NULL) {
        printf("mkdir: %s: No such file or directory\n", directory_path);
    } else if (find_file_from_parent(parent_directory, directory_name) != NULL) {
        printf("mkdir: %s: File exists\n", directory_path);
    } else if (strlen(directory_name) > FILE_NAME_LENGTH) {
        printf("%s: Too long directory name\n", directory_path);
    } else if (!is_legal_file_name(directory_name)) {
        printf("mkdir: %s: Illegal filename\n", directory_path);
        return;
    } else {
        
        struct child_file_t directory;
        memset(&directory.filename, '\0', FILE_NAME_LENGTH);
        directory.inode_number = get_free_inode();
        strcpy(directory.filename, directory_name);
        
        struct inode_t* inode = get_inode_by_num(directory.inode_number);
        inode->link_count = 1;
        inode->mode = ISDIR + MAX_DIRECTORY_PERMISSION - superblock.umask;
        strcpy(inode->user, current_user);
        strcpy(inode->group, current_group);
        inode->size = 0;
        inode->created_time = time(NULL);
        inode->modified_time = time(NULL);
        inode->accessed_time = time(NULL);
        
        unsigned short writing_block = (unsigned short)(parent_directory->size / BLOCK_SIZE);
        size_t writing_position =parent_directory->size % BLOCK_SIZE;
        
        if (writing_block < NADDR - 2) {
            if (writing_position == 0) {
                parent_directory->data_address[writing_block] = get_free_data_block();
                if (parent_directory->data_address[writing_block] == BLOCK_NUM) {
                    return_inode(inode->number);
                    return;
                }
            }
            fseek(disk, (parent_directory->data_address[writing_block] + DATA_BLOCK_START) * BLOCK_SIZE + writing_position, SEEK_SET);
            fwrite(&directory, sizeof(struct child_file_t), 1, disk);
            parent_directory->size += sizeof(struct child_file_t);
            
            char* dot_path = (char*)malloc(strlen(directory_name) + 4);
            strcpy(dot_path, directory_name);
            strcat(dot_path, "/.");
            link_file(parent_directory, dot_path, directory_name);
            strcat(dot_path, ".");
            link_file(parent_directory, dot_path, ".");
        }
    }
}

void remove_directory(char* path) {
    if (path == NULL) {
        return;
    } else if (strcmp(path + strlen(path) - 2, "/.")) {
        printf("rmdir: %s: Invalid argument\n", path);
        return;
    }
    struct inode_t* parent_directory = find_parent(current_working_inode, path);
    if (parent_directory == NULL) {
        printf("rmdir: %s: No such file or directory\n", path);
    }
    char* filename = (char*)malloc(strlen(path));
    get_file_name(path, filename);
    struct inode_t* file = find_file_from_parent(parent_directory, filename);
    if (file == NULL) {
        printf("rmdir: %s: No such file or directory\n", path);
    }
    if ((file->mode & 07000) != ISDIR) {
        printf("rmdir: %s: Not a directory\n", path);
    } else if (!check_write_permission(file)) {
        printf("rmdir: %s: Permission denied\n", path);
    } else if (file->size == sizeof(struct child_file_t) * 2) {
        size_t parent_size = parent_directory->size;
        char *data = (char *)malloc(parent_size);
        read_data(parent_directory, data);
        struct child_file_t* directory_content = (struct child_file_t*)data;
        for (unsigned i = 0; i < parent_size / sizeof(struct child_file_t); ++i) {
            if (strcmp(directory_content[i].filename, filename) == 0) {
                directory_content[i] = directory_content[parent_size / sizeof(struct child_file_t) - 1];
                erase_data(parent_directory);
                write_data(parent_directory, data, parent_size - sizeof(struct child_file_t));
            }
        }

        --file->link_count;
        --file->link_count;
        --parent_directory->link_count;
        if (file->link_count == 0) {
            erase_data(file);
            return_inode(file->number);
        }
    } else {
        printf("rmdir: %s: Directory not empty\n", path);
    }
}

void touch_file(char* path) {
    if (path == NULL) {
        return;
    }
    struct inode_t* parent_directory;
    char* filename = (char*)malloc(strlen(path));
    if (strchr(path, '/') != NULL) {
        parent_directory = find_parent(current_working_inode, path);
        get_file_name(path, filename);
    } else {
        parent_directory = current_working_inode;
        strcpy(filename, path);
    }
    if (parent_directory == NULL) {
        printf("touch: %s: No such file or directory\n", path);
    } else if (find_file_from_parent(parent_directory, filename) != NULL) {
        struct inode_t* inode = find_file_from_parent(parent_directory, filename);
        inode->modified_time = time(NULL);
        inode->accessed_time = time(NULL);
    } else if (strlen(filename) > FILE_NAME_LENGTH) {
        printf("%s: Too long file name\n", path);
    } else if (!is_legal_file_name(filename)) {
        printf("touch: %s: Illegal filename\n", filename);
        return;
    } else {
        
        struct child_file_t file;
        memset(&file.filename, '\0', FILE_NAME_LENGTH);
        file.inode_number = get_free_inode();
        strcpy(file.filename, filename);
        
        struct inode_t* inode = get_inode_by_num(file.inode_number);
        inode->link_count = 1;
        inode->mode = ISREG + MAX_FILE_PERMISSION - superblock.umask;
        strcpy(inode->user, current_user);
        strcpy(inode->group, current_group);
        inode->size = 0;
        inode->created_time = time(NULL);
        inode->modified_time = time(NULL);
        inode->accessed_time = time(NULL);
        
        unsigned short writing_block = (unsigned short)(parent_directory->size / BLOCK_SIZE);
        size_t writing_position =parent_directory->size % BLOCK_SIZE;
        
        if (writing_block < NADDR - 2) {
            if (writing_position == 0) {
                parent_directory->data_address[writing_block] = get_free_data_block();
                if (parent_directory->data_address[writing_block] == BLOCK_NUM) {
                    return_inode(inode->number);
                    return;
                }
            }
            fseek(disk, (parent_directory->data_address[writing_block] + DATA_BLOCK_START) * BLOCK_SIZE + writing_position, SEEK_SET);
            fwrite(&file, sizeof(struct child_file_t), 1, disk);
            parent_directory->size += sizeof(struct child_file_t);
        }
    }
}

void resize_text_file(struct inode_t* inode, size_t new_size) {
    if (!check_write_permission(inode)) {
        printf("edit: Permission denied\n");
    }
    erase_data(inode);
    char* data = (char*)malloc(new_size + 1);
    memset(data, 'a', new_size);
    write_data(inode, data, new_size);
}

void remove_regular_file(char* path) {
    if (path == NULL) {
        return;
    }
    struct inode_t* parent_directory = find_parent(current_working_inode, path);
    if (parent_directory == NULL) {
        printf("rm: %s: No such file or directory\n", path);
    }
    char* filename = (char*)malloc(strlen(path));
    get_file_name(path, filename);
    struct inode_t* file = find_file_from_parent(parent_directory, filename);
    if (file == NULL) {
        printf("rm: %s: No such file or directory\n", path);
    }
    if ((file->mode & 07000) == ISDIR) {
        printf("rm: %s: is a directory\n", path);
    } else if (!check_write_permission(file)) {
        printf("rm: %s: Permission denied\n", path);
    } else {
        size_t parent_size = parent_directory->size;
        char *data = (char *)malloc(parent_size);
        read_data(parent_directory, data);
        struct child_file_t* directory_content = (struct child_file_t*)data;
        for (unsigned i = 0; i < parent_size / sizeof(struct child_file_t); ++i) {
            if (strcmp(directory_content[i].filename, filename) == 0) {
                directory_content[i] = directory_content[parent_size / sizeof(struct child_file_t) - 1];
                erase_data(parent_directory);
                write_data(parent_directory, data, parent_size - sizeof(struct child_file_t));
            }
        }

        --file->link_count;
        if (file->link_count == 0) {
            erase_data(file);
            return_inode(file->number);
        }
    }
}

void cat(char* path) {
    if (path == NULL) {
        return;
    }
    struct inode_t* inode = find_file_by_path(current_working_inode, path);
    if (inode == NULL) {
        printf("cat: %s: No such file or directory\n", path);
    } else if ((inode->mode & 07000) == ISDIR) {
        printf("cat: %s: Is a directory\n", path);
    } else if (!check_read_permission(inode)) {
        printf("cat: %s: Permission denied\n", path);
    } else {
        char* data = (char*)malloc(inode->size);
        read_data(inode, data);
        for (size_t i = 0; i < inode->size; ++i) {
            putchar(data[i]);
        }
        putchar('\n');
    }
}



void show_umask() {
    printf("%04o\n", superblock.umask);
}

void change_umask(unsigned short new_umask) {
    if (new_umask < 01000) {
        superblock.umask = new_umask;
    } else {
        printf("umask: %o: octal number out of range\n", new_umask);
    }
}
#endif
