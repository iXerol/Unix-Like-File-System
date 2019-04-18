#ifndef strings_h
#define strings_h

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

void get_parent_path(const char* path, char* parent_path);

void get_file_name(const char* path, char* filename);

void split_relative_path(const char* path, char* child, char* child_path);

bool is_legal_file_name(const char* filename);

unsigned int string_to_octal(const char* string);

bool start_with(const char* string, const char* start, char* parameters);

void split_parameters(const char* parameters, char* first_parameter, char* other_parameters);

#endif /* strings_h */
