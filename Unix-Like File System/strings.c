#include "strings.h"

void get_parent_path(const char* path, char* parent_path) {
    if (parent_path == NULL) {
        return;
    }
    if (path == NULL) {
        strcpy(parent_path, "");
        return;
    }
    char* last_slash = strrchr(path, '/');
    if (last_slash == NULL) {
        return;
    }
    strncpy(parent_path, path, last_slash - path);
    parent_path[path - last_slash] = '\0';
    return;
}

void get_file_name(const char* path, char* filename) {
    if (filename == NULL) {
        return;
    }
    if (path == NULL) {
        strcpy(filename, "");
        return;
        //若 path 為空指針，則將 filename 置為空串
    }
    char* last_slash = strrchr(path, '/');
    if (last_slash == NULL) {
        strcpy(filename, path);
        //若 path 中無‘/’，則直接拷貝 path 至 filename
    } else if (strlen(last_slash) == 1) {
        strcpy(filename, "");
        //若 path 中‘/’後無字符，則將 filename 置為空串
    } else {
        strcpy(filename, last_slash + 1);
        //從 path 中最後出現的‘/’後一字符開始拷貝至 filename
    }
}

void split_relative_path(const char* path, char* child, char* child_path) {
    if (child == NULL || child_path == NULL) {
        return;
    }
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
        strncpy(child, path, first_slash - path);
        child[first_slash - path] = '\0';
        strcpy(child_path, first_slash + 1);
        //若 path 中有‘/’，則將其前後分別拷貝給 child 與 child_path
    }
}

bool is_legal_file_name(const char* filename) {
    if (filename == NULL) {
        return false;
    }
    size_t length = strlen(filename);
    if (length == 0) {
        return false;
    }
    for (size_t i = 0; i < length; ++i) {
        if (!isalnum(filename[i]) && filename[i] != '.') {
            return false;
        }
    }
    return true;
}

unsigned int string_to_octal(const char* string) {
    unsigned int num = 0;
    if (string != NULL) {
        size_t length = strlen(string);
        for (size_t i = 0; i < length; ++i) {
            if (string[i] < '0' || string[i] > '7') {
                return num;
            }
            num = (num << 3) + (string[i] - '0');
        }
    }
    return num;
}

// 判斷命令是否以特定字符串開始，若是則將之後的參數分離至 parameters
bool start_with(const char* string, const char* start, char* parameters) {
    if(string == NULL || start == NULL) {
        return false;
    }
    size_t string_length = strlen(string);
    size_t start_length = strlen(start);
    size_t blank_num = 0;
    size_t i;
    while (blank_num < string_length && isspace(string[blank_num])) {
        ++blank_num;
    }
    if (blank_num + start_length > string_length) {
        return false;
    }
    for (i = 0 ; i < start_length; ++i) {
        if (string[i + blank_num] != start[i]) {
            return false;
        }
    }
    if (i + blank_num < string_length && !isspace(string[i + blank_num])) {
        return false;
    }
    if (parameters != NULL) {
        strcpy(parameters, string + start_length);
    }
    return true;
}

void split_parameters(const char* parameters, char* first_parameter, char* other_parameters) {
    if (first_parameter == NULL || other_parameters == NULL) {
        return;
    }
    if (parameters == NULL) {
        strcpy(first_parameter, "");
        strcpy(other_parameters, "");
        return;
        //若 parameters 為空指針，則將 first_parameter 與 other_parameters 置空
    }

    size_t length = strlen(parameters);
    size_t blank_num = 0;
    size_t i;
    while (blank_num < length && isspace(parameters[blank_num])) {
        ++blank_num;
    }
    for (i = 0 ; i < length - blank_num && !isspace(parameters[i + blank_num]); ++i);
    strncpy(first_parameter, parameters + blank_num, i);
    first_parameter[i] = '\0';
    strcpy(other_parameters, parameters + blank_num + i);
}
