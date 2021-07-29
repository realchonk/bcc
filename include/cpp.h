#ifndef FILE_CPP_H
#define FILE_CPP_H
#include <stdio.h>

extern const char* cpp_path;
extern char** predef_macros;
extern char** includes;
FILE* run_cpp(const char* source_name);

#endif /* FILE_CPP_H */
