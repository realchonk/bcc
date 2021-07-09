#include "cpp.h"

const char* source_name = NULL;

char** read_lines(FILE* file) {
   char** buf = NULL;
   char* line = NULL;
   int ch;
   size_t row = 0, col = 0;
   while ((ch = fgetc(file)) != EOF) {
   begin:;
      if (ch == '\n') {
         buf_push(line, '\0');
         buf_push(buf, line);
         line = NULL;
         ++row;
         col = 0;
         continue;
      }
      if (ch == '\\') {
         ch = fgetc(file);
         if (ch == '\n') {
            ++row;
            col = 0;
            continue;
         }
         else if (ch == EOF) {
            fprintf(stderr, "%s:%zu:%zu: backslash at end of file\n", source_name, row + 1, col + 1);
            break;
         }
         buf_push(line, '\\');
         ++col;
         goto begin;
      }
      buf_push(line, ch);
      ++col;
   }
   return buf;
}
void free_lines(char** lines) {
   for (size_t i = 0; i < buf_len(lines); ++i) {
      buf_free(lines[i]);
   }
   buf_free(lines);
}
void print_lines(FILE* file, char** lines) {
   for (size_t i = 0; i < buf_len(lines); ++i) {
      fprintf(file, "%s\n", lines[i]);
   }
}

