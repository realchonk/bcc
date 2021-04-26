#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include "strint.h"
#include "lex.h"
#include "buf.h"

static FILE* file = NULL;
static char peekd_ch = '\0';
static struct token peekd_tk;
static struct source_pos pos;

// INPUT STUFF

static int input_read(void) {
   const char ch = fgetc(file);
   if (ch == '\n') {
      pos.line += 1;
      pos.column = 0;
   } else ++pos.column;
   return ch;
}

static int input_next(void) {
   if (peekd_ch) {
      const char tmp = peekd_ch;
      peekd_ch = '\0';
      return tmp;
   } else return input_read();
}
#define input_peek() (peekd_ch ? peekd_ch : (peekd_ch = input_read()))
#define input_skip() (peekd_ch ? peekd_ch = '\0' : (input_read(), 0))

// LEXER STUFF

void lexer_init(FILE* f, const char* fn) {
   if (file) fclose(file);
   file = f;
   peekd_ch = '\0';
   peekd_tk.type = TK_DUMMY;
   pos.file = fn;
   pos.line = 0;
   pos.column = 0;
}

static struct token lexer_impl(void);

struct token lexer_peek(void) {
   return peekd_tk.type ? peekd_tk : (peekd_tk = lexer_impl());
}
struct token lexer_next(void) {
   if (peekd_tk.type) {
      const struct token tk = peekd_tk;
      peekd_tk.type = TK_DUMMY;
      return tk;
   } else return lexer_impl();
}

bool lexer_eof(void) {
   return feof(file) && !peekd_ch && !peekd_tk.type;
}

bool lexer_matches(enum token_type type) {
   return lexer_peek().type == type;
}
bool lexer_match(enum token_type type) {
   if (lexer_matches(type)) return peekd_tk.type = TK_DUMMY, true;
   else return false;
}
struct token lexer_expect(enum token_type type) {
   const struct token tk = lexer_next();
   if (tk.type == type) return tk;
   // TODO: implement error()
   printf("expected %s got %s\n", token_type_str[type], token_type_str[tk.type]);
   abort();
}

#define skip_ws() while (isspace(input_peek())) input_skip()

static int xctoi(char ch) {
   if (isdigit(ch)) return ch - '0';
   else if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
   else if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
   else return -1;
}
static bool isodigit(char ch) {
   return ch >= '0' && ch <= '7';
}

static char parse_escape_seq(void) {
   char result, ch = input_next();
   if (isodigit(ch)) {
      result = ch - '0';
      for (int i = 0; i < 2 && isodigit(input_peek()); ++i)
         result = result * 8 + (input_next() - '0');
      return result;
   }
   switch (ch) {
   case 'a':   return '\a';
   case 'b':   return '\b';
   case 'f':   return '\f';
   case 'n':   return '\n';
   case 'r':   return '\r';
   case 't':   return '\t';
   case 'v':   return '\v';
   case 'x':
      result = 0;
      ch = input_next();
      if (!isxdigit(ch)) {
         // TODO: error()
      }
      result = xctoi(ch);
      if (isxdigit(input_peek())) {
         result *= 16;
         result += xctoi(input_next());
      }
      return result;
   case '\\':  return '\\';
   case '\"':  return '\"';
   case '\'':  return '\'';
   default:    // TODO: error()
      return 0;
   }
}

static struct token lexer_impl(void) {
   int ch;
   skip_ws();
   const struct source_pos start = pos;
   ch = input_peek();
   if (isdigit(ch)) {
      uintmax_t iVal = 0;
      while (isdigit(input_peek())) iVal = iVal * 10 + (input_next() - '0');
      if (input_peek() == '.') {
         fpmax_t fVal = 0.0;
         input_skip();
         int exp = 0;
         while (isdigit(input_peek())) fVal = fVal + ((input_next() - '0') * powl(10, --exp));
         if (input_peek() == 'e') {
            // TODO: add scientific-notation
         }
         return (struct token){ TK_FLOAT, start, .fVal = iVal + fVal };
      }
      return (struct token){ TK_INTEGER, start, .iVal = iVal };
   } else if (isalpha(ch) || ch == '_') {
      char* buf = NULL;
      while ((ch = input_peek()) && (isalnum(ch) || ch == '_')) {
         buf_push(buf, ch);
      }
      // TODO: implement keyword-checking
      const istr_t s = strnint(buf, buf_len(buf));
      buf_free(buf);
      return (struct token){ TK_NAME, start, .str = s };
   } else if (ch == '"') {
      char* buf = NULL;
      input_skip();
      while ((ch = input_next()) != '"') {
         if (ch == '\\') ch = parse_escape_seq();
         buf_push(buf, ch);
      }
      const istr_t s = strnint(buf, buf_len(buf));
      buf_free(buf);
      return (struct token){ TK_STRING, start, .str = s };
   } else if (ch == '\'') {
      input_skip();
      ch = input_next();
      if (ch == '\''); // TODO: error()
      if (ch == '\\') ch = parse_escape_seq();
      if (input_next() != '\'') {
         // TODO: error()
      }
      return (struct token){ TK_CHARACTER, start, .ch = ch };
   } else {
      input_skip();
      switch (ch) {

      case EOF: return (struct token){ TK_EOF, start };
      }
   }
}

