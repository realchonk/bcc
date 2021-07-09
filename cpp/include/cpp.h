#ifndef FILE_CPP_H
#define FILE_CPP_H
#include <stdio.h>
#include "buf.h"

extern const char* source_name;

int run_cpp(FILE* in, FILE* out);
char** read_lines(FILE* file);
void print_lines(FILE* out, char**);
void free_lines(char**);

#endif /* FILE_CPP_H */
