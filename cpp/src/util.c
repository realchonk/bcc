#include "cpp.h"

const char* source_name = NULL;

static int read_char2(FILE* file, size_t* linenum) {
   const int ch = fgetc(file);
   if (ch == '\n') {
      ++*linenum;
   }
   return ch;
}

static int read_char(FILE* file, size_t* linenum) {
   int ch = read_char2(file, linenum);
   if (ch == '/') {
      ch = fgetc(file);
      if (ch == '/') {
         while (!feof(file) && read_char2(file, linenum) != '\n');
         return read_char(file, linenum);
      } else if (ch == '*') {
         while (!feof(file)) {
            ch = read_char2(file, linenum);
            if (read_char2(file, linenum) == '*' && read_char2(file, linenum) == '/')
               return read_char(file, linenum);
         }
         return EOF;
      } else {
         return ungetc(ch, file), '/';
      }
   } else return ch;
}

static char* read_line(FILE* file, bool* eof, size_t* linenum) {
   char* line = NULL;
   int ch;
   while ((ch = read_char(file, linenum)) != EOF) {
   begin:;
      if (ch == '\n')
         break;
      else if (ch == '\\') {
         ch = read_char2(file, linenum);
         if (ch == '\n')
            continue;
         else if (ch == EOF) {
            warn(*linenum, "'\\' at end of file");
            *eof = true;
            break;
         }
         buf_push(line, '\\');
         goto begin;
      } else buf_push(line, ch);
   }
   if (line)
      buf_push(line, '\0');
   *eof |= (ch == EOF);
   return line;
}

struct line_pair* read_lines(FILE* file) {
   struct line_pair* pairs = NULL;
   bool eof = false;
   size_t linenum = 0;
   do {
      struct line_pair pair;
      pair.linenum = linenum;
      pair.line = read_line(file, &eof, &linenum);
      if (pair.line)
         buf_push(pairs, pair);
   } while (!eof);
   return pairs;
}

void free_lines(struct line_pair* lines) {
   for (size_t i = 0; i < buf_len(lines); ++i) {
      buf_free(lines[i].line);
   }
   buf_free(lines);
}
void print_lines(FILE* file, const struct line_pair* lines) {
   for (size_t i = 0; i < buf_len(lines); ++i) {
      fprintf(file, "%s\n", lines[i].line);
   }
}

