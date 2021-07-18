#include "cpp.h"

const char* source_name = NULL;

static int read_char(FILE* file) {
   int ch = fgetc(file);
   if (ch == '/') {
      ch = fgetc(file);
      if (ch == '/') {
         while (!feof(file) && fgetc(file) != '\n');
         return read_char(file);
      } else if (ch == '*') {
         while (!feof(file)) {
            if (fgetc(file) == '*' && fgetc(file) == '/')
               return read_char(file);
         }
         return EOF;
      } else {
         return ungetc(ch, file), '/';
      }
   } else return ch;
}

char** read_lines(FILE* file) {
   char** buf = NULL;
   char* line = NULL;
   int ch;
   size_t row = 0, col = 0;
   while ((ch = read_char(file)) != EOF) {
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
   if (line) {
      buf_push(line, '\0');
      buf_push(buf, line);
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

