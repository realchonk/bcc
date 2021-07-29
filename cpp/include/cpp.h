#ifndef FILE_CPP_H
#define FILE_CPP_H
#include <stdbool.h>
#include <stdio.h>
#include "buf.h"

extern const char* source_name;
extern bool console_color;
extern bool failed;

struct line_pair {
   char* line;
   size_t linenum;
};

int run_cpp(FILE* in, FILE* out);
struct line_pair* read_lines(FILE* file);
void print_lines(FILE* out, const struct line_pair*);
void free_lines(struct line_pair*);

void warn(size_t linenum, const char*, ...);
void fail(size_t linenum, const char*, ...);

int eval(size_t linenum, const char* s);

#endif /* FILE_CPP_H */
