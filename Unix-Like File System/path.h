#ifndef path_h
#define path_h

#include <stdlib.h>
#include <string.h>

void remove_tail_slashes(char* new_path, char* path) {
    if (path == NULL || new_path == NULL) {
        return;
    }
    strcpy(new_path, "");
    size_t i;
    for (i = strlen(path); i > 0; --i) {
        if (path[i- 1] != '/') {
            break;
        }
    }
    strncpy(new_path, path, i);
}

void get_file_name(char* path, char* filename) {
    if (path == NULL) {
        strcpy(filename, "");
        return;
        //若 path 為空指針，則將 filename 置為空串
    }
    char* new_path = (char*)malloc(strlen(path));
    remove_tail_slashes(new_path, path);
    char* last_slash = strrchr(new_path, '/');
    if (last_slash == NULL) {
        if (strlen(new_path) > 0) {
            strcpy(filename, new_path);
            //若 path 中無‘/’，則直接拷貝 path 至 filename
        } else {
            strcpy(filename, "");
        }
    } else if (strlen(last_slash) == 1) {
        strcpy(filename, "");
        //若 path 中‘/’後無字符，則將 filename 置為空串
    } else {
        strcpy(filename, last_slash + 1);
        //從 path 中最後出現的‘/’後一字符開始拷貝至 filename
    }
}

void split_relative_path(char* path, char* child, char* child_path) {
    if (path == NULL) {
        strcpy(child, "");
        strcpy(child_path, "");
        return;
        //若 path 為空指針，則將 child 與 child_path 置空
    }
    char* first_slash = strchr(path, '/');
    if (first_slash == NULL) {
        if (strlen(path) > 0) {
            strcpy(child, path);
            strcpy(child_path, "");
            //若 path 中無‘/’，則直接拷貝 path 至 child，並將 child_path 置空
        } else {
            strcpy(child, "");
            strcpy(child_path, "");
            //若 path 為空串，則將 child 與 child_path 置空
        }
    } else {
        strncpy(child, path, strlen(path) - strlen(first_slash));
        strcpy(child + strlen(path) - strlen(first_slash), "");
        strcpy(child_path, first_slash + 1);
        //若 path 中有‘/’，則將其前後分別拷貝給 child 與 child_path
    }
}

#endif /* path_h */
