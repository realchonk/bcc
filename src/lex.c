#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include "strint.h"
#include "error.h"
#include "lex.h"
#include "buf.h"

static FILE* file = NULL;
static int peekd_ch = '\0';
static struct token peekd_tk;
static struct source_pos pos;

// INPUT STUFF

#define input_peek() (peekd_ch ? peekd_ch : (peekd_ch = input_read()))
#define input_skip() (peekd_ch ? peekd_ch = '\0' : (input_read(), 0))
static int input_read(void) {
   const int ch = fgetc(file);
   if (ch == '\n') {
      pos.line += 1;
      pos.column = 0;
   } else ++pos.column;
   return ch;
}

static int input_next(void) {
   if (peekd_ch) {
      const int tmp = peekd_ch;
      peekd_ch = '\0';
      return tmp;
   } else return input_read();
}
static bool input_match(int ch) {
   if (input_peek() == ch) return input_skip(), true;
   else return false;
}

#define lex_error(...) parse_error(&pos, __VA_ARGS__)

// LEXER STUFF

void lexer_init(FILE* f, const char* fn) {
   token_init();
   if (file) fclose(file);
   file = f;
   peekd_ch = '\0';
   peekd_tk.type = TK_DUMMY;
   pos.file = fn;
   pos.line = 0;
   pos.column = 0;
}
void lexer_free(void) {
   if (file) {
      fclose(file);
      file = NULL;
   }
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
void lexer_skip(void) {
   if (peekd_tk.type) peekd_tk.type = TK_DUMMY;
   else lexer_impl();
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
   else lex_error("expected %s, got %s", token_type_str[type], token_type_str[tk.type]);
}

#define skip_ws() while (isspace(input_peek())) input_skip()

static int xctoi(int ch) {
   if (isdigit(ch)) return ch - '0';
   else if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
   else if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
   else lex_error("expected hexadecimal digit, got '%c'", ch);
}
static bool isodigit(int ch) {
   return ch >= '0' && ch <= '7';
}

static int parse_escape_seq(void) {
   int result, ch = input_next();
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
      result = xctoi(ch);
      if (isxdigit(input_peek())) {
         result *= 16;
         result += xctoi(input_next());
      }
      return result;
   case '\\':  return '\\';
   case '\"':  return '\"';
   case '\'':  return '\'';
   default:    lex_error("expected escape sequence, got '\\%c'", ch);
   }
}

static struct token lexer_impl(void) {
   int ch;
   skip_ws();
   const struct source_pos start = pos;
   ch = input_peek();
   if (isdigit(ch)) {
      uintmax_t iVal = 0;
      if (ch == '0') {
         input_skip();
         ch = input_peek();
         if (ch == 'x' || ch == 'X') {
            input_skip();
            while (isxdigit(input_peek())) iVal = iVal * 16 + xctoi(input_next());
         } else if (isodigit(ch)) {
            while (isodigit(input_peek())) iVal = iVal * 8 + (input_next() - '0');
         }
         return (struct token){ .type = TK_INTEGER, start, pos, .iVal = iVal };
      }
      while (isdigit(input_peek())) iVal = iVal * 10 + (input_next() - '0');
#if !DISABLE_FP
      if (input_match('.')) {
         fpmax_t fVal = 0.0;
         int exp = 0;
         while (isdigit(input_peek())) fVal = fVal + ((input_next() - '0') * powl(10, --exp));
         fVal += iVal;
         if (input_match('e')) {
            int sign = 1;
            if (input_match('-')) sign = -1;
            else input_match('+');
            exp = 0;
            if (!isdigit(input_peek())) lex_error("expected intger, got '%c'", input_peek());
            while (isdigit(input_peek())) exp = exp * 10 + (input_next() - '0');
            fVal *= powl(10.0, sign * exp);
         }
         return (struct token){ TK_FLOAT, start, pos, .fVal = fVal };
      } else if (input_match('e')) {
         int sign = 1, exp = 0;
         if (input_match('-')) sign = -1;
         else input_match('+');
         if (!isdigit(input_peek())) lex_error("expected intger, got '%c'", input_peek());
         while (isdigit(input_peek())) exp = exp * 10 + (input_next() - '0');
         return (struct token){ TK_FLOAT, start, pos, .fVal = (fpmax_t)iVal * powl(10.0, sign * exp) };
      }
#endif
      return (struct token){ TK_INTEGER, start, pos, .iVal = iVal };
   } else if (isalpha(ch) || ch == '_') {
      char* buf = NULL;
      while ((ch = input_peek()) && (isalnum(ch) || ch == '_')) {
         buf_push(buf, ch);
         input_skip();
      }
      const istr_t s = strnint(buf, buf_len(buf));
      for (int i = TK_EOF + 1; i < NUM_TOKENS; ++i) {
         if (s == token_type_str[i])
            return (struct token){ i, start, pos, 0 };
      }

      buf_free(buf);
      return (struct token){ TK_NAME, start, pos, .str = s };
   } else if (ch == '"') {
      char* buf = NULL;
      input_skip();
      while ((ch = input_next()) != '"') {
         if (ch == '\\') ch = parse_escape_seq();
         buf_push(buf, ch);
      }
      const istr_t s = strnint(buf, buf_len(buf));
      buf_free(buf);
      return (struct token){ TK_STRING, start, pos, .str = s };
   } else if (ch == '\'') {
      input_skip();
      ch = input_next();
      if (ch == '\'') lex_error("empty character literal");
      if (ch == '\\') ch = parse_escape_seq();
      if (input_next() != '\'') lex_error("missing terminating ' character");
      return (struct token){ TK_CHARACTER, start, pos, .ch = ch };
   } else {
      input_skip();
      switch (ch) {
      case '(':   return (struct token){ TK_LPAREN, start, pos, 0 };
      case ')':   return (struct token){ TK_RPAREN, start, pos, 0 };
      case '[':   return (struct token){ TK_LBRACK, start, pos, 0 };
      case ']':   return (struct token){ TK_RBRACK, start, pos, 0 };
      case '{':   return (struct token){ TK_CLPAREN, start, pos, 0 };
      case '}':   return (struct token){ TK_CRPAREN, start, pos, 0 };
      case ',':   return (struct token){ TK_COMMA, start, pos, 0 };
      case ':':   return (struct token){ TK_COLON, start, pos, 0 };
      case ';':   return (struct token){ TK_SEMICOLON, start, pos, 0 };
      case '?':   return (struct token){ TK_QMARK, start, pos, 0 };
      case '~':   return (struct token){ TK_WAVE, start, pos, 0 };
      case '+':
         if (input_match('+')) return (struct token){ TK_PLPL, start, pos, 0 };
         else if (input_match('=')) return (struct token){ TK_PLEQ, start, pos, 0 };
         else return (struct token){ TK_PLUS, start, pos, 0 };
      case '-':
         if (input_match('-')) return (struct token){ TK_MIMI, start, pos, 0 };
         else if (input_match('=')) return (struct token){ TK_MIEQ, start, pos, 0 };
         else if (input_match('>')) return (struct token){ TK_ARROW, start, pos, 0 };
         else return (struct token){ TK_MINUS, start, pos, 0 };
      case '*':
         if (input_match('=')) return (struct token){ TK_STEQ, start, pos, 0 };
         else return (struct token){ TK_STAR, start, pos, 0 };
      case '/':
         if (input_match('=')) return (struct token){ TK_SLEQ, start, pos, 0 };
         else if (input_match('/')) {
            while (input_next() != '\n');
            return lexer_impl();
         } else if (input_match('*')) {
            while (1) {
               if (input_match('*') && input_match('/')) break;
               else input_skip();
            }
            return lexer_impl();
         } else return (struct token){ TK_SLASH, start, pos, 0 };
      case '!':
         if (input_match('=')) return (struct token){ TK_NEQ, start, pos, 0 };
         else return (struct token){ TK_NOT, start, pos, 0 };
      case '=':
         if (input_match('=')) return (struct token){ TK_EQEQ, start, pos, 0 };
         else return (struct token){ TK_EQ, start, pos, 0 };
      case '>':
         if (input_match('=')) return (struct token){ TK_GREQ, start, pos, 0 };
         else if (input_match('>')) {
            if (input_match('=')) return (struct token){ TK_GRGREQ, start, pos, 0 };
            else return (struct token){ TK_GRGR, start, pos, 0 };
         } else return (struct token){ TK_GR, start, pos, 0 };
      case '<':
         if (input_match('=')) return (struct token){ TK_LEEQ, start, pos, 0 };
         else if (input_match('<')) {
            if (input_match('=')) return (struct token){ TK_LELEEQ, start, pos, 0 };
            else return (struct token){ TK_LELE, start, pos, 0 };
         } else return (struct token){ TK_LE, start, pos, 0 };
      case '&':
         if (input_match('&')) return (struct token){ TK_AMPAMP, start, pos, 0 };
         else if (input_match('=')) return (struct token){ TK_AMPEQ, start, pos, 0 };
         else return (struct token){ TK_AMP, start, pos, 0 };
      case '|':
         if (input_match('|')) return (struct token){ TK_PIPI, start, pos, 0 };
         else if (input_match('=')) return (struct token){ TK_PIPEEQ, start, pos, 0 };
         else return (struct token){ TK_PIPE, start, pos, 0 };
      case '^':
         if (input_match('=')) return (struct token){ TK_XOREQ, start, pos, 0 };
         else return (struct token){ TK_XOR, start, pos, 0 };
      case '%':
         if (input_match('=')) return (struct token){ TK_PERCEQ, start, pos, 0 };
         else return (struct token){ TK_PERC, start, pos, 0 };
      case '.':
         if (input_match('.')) {
            if (input_next() == '.') return (struct token){ TK_DDD, start, pos, 0 };
            else lex_error("expected '.'");
         } else return (struct token){ TK_DOT, start, pos, 0 };

      case EOF:   return (struct token){ TK_EOF, start, pos, 0 };
      default:    lex_error("illegal input character '%c'", ch);
      }
   }
}

