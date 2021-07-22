#ifndef FILE_CPP_H
#define FILE_CPP_H
#include <stdbool.h>
#include <stdio.h>
#include "buf.h"

extern const char* source_name;
extern bool console_color;

int run_cpp(FILE* in, FILE* out);
char** read_lines(FILE* file);
void print_lines(FILE* out, char**);
void free_lines(char**);

void warn(size_t linenum, const char*, ...);

#endif /* FILE_CPP_H */
