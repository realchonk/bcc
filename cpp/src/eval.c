#include <ctype.h>
#include "macro.h"
#include "cpp.h"

#define matches(ch) (*str == (ch))
#define match(ch) (matches(ch) ? ++str, true : false)
#define isname1(ch) (isalpha(ch) || (ch) == '_')
#define isname(ch) (isname1(ch) || isdigit(ch))

static void skip_ws(const char** str) {
   while (isspace(**str)) ++*str;
}

static int do_eval(size_t linenum, const char** str);

static const char* defined(void) {
   static const char* str = NULL;
   return str ? str : (str = strint("defined"));
}

static int prim(size_t linenum, const char** str) {
   skip_ws(str);
   if (**str == '(') {
      ++*str;
      skip_ws(str);
      const int i = do_eval(linenum, str);
      skip_ws(str);
      if (**str != ')') {
         fail(linenum, "missing ')' in expression");
         return 0;
      }
      ++*str;
      return i;
   } else if (isdigit(**str)) {
      int i = 0;
      while (isdigit(**str))
         i = i * 10 + (*(*str)++ - '0');
      skip_ws(str);
      return i;
   } else if (isname1(**str)) {
      const char* begin = *str;
      while (isname(**str))
         ++*str;
      istr_t name = strrint(begin, *str);
      skip_ws(str);
      if (name == defined()) {
         const bool has_paren = **str == '(';
         if (has_paren) {
            ++*str;
            skip_ws(str);
         }
         if (!isname1(**str)) {
            fail(linenum, "operator 'defined' expects an identifier");
            return 0;
         }
         begin = *str;
         while (isname(**str))
            ++*str;
         name = strrint(begin, *str);
         skip_ws(str);
         if (has_paren) {
            if (**str != ')') {
               fail(linenum, "missing ')'");
               return 0;
            } else ++*str;
         }
         return get_macro(name) != NULL;
      }
      if (**str == '(') {
         ++*str;
         // TODO: implement macro functions
         fail(linenum, "macro functions are currently not implemented");
         return 0;
      }
      const struct macro* m = get_macro(name);
      if (m)
         return eval(linenum, m->text);
      else return 0;
   } else if (**str == '"') {
      fail(linenum, "strings are not valid preprocessor expressions");
      return 0;
   } else {
      fail(linenum, "invalid character: '%c'", *str);
      return 0;
   }
}

static int unary(size_t linenum, const char** str) {
   skip_ws(str);
   switch (**str) {
   case '+':
      ++*str;
      skip_ws(str);
      return unary(linenum, str);
   case '-':
      ++*str;
      skip_ws(str);
      return -unary(linenum, str);
   case '!':
      ++*str;
      skip_ws(str);
      return !unary(linenum, str);
   case '~':
      ++*str;
      skip_ws(str);
      return ~unary(linenum, str);
   default:
      return prim(linenum, str);
   }
}

static int mult(size_t linenum, const char** str) {
   int left = unary(linenum, str);
   while (skip_ws(str), **str == '*' || **str == '/' || **str == '%') {
      const char op = **str;
      ++*str;
      const int right = unary(linenum, str);
      switch (op) {
      case '*':   left *= right; break;
      case '/':   left /= right; break;
      case '%':   left %= right; break;
      default:
         fail(linenum, "invalid operator '%c'", op);
         return 0;
      }
   }
   return left;
}
static int add(size_t linenum, const char** str) {
   int left = mult(linenum, str);
   while (skip_ws(str), **str == '+' || **str == '-') {
      const char op = **str;
      ++*str;
      const int right = mult(linenum, str);
      switch (op) {
      case '+':   left += right; break;
      case '-':   left -= right; break;
      default:
         fail(linenum, "invalid operator '%c'", op);
         return 0;
      }
   }
   return left;
}
static int shift(size_t linenum, const char** str) {
   int left = add(linenum, str);
   while (skip_ws(str), (**str == '<' || **str == '>') && (*str)[1] == **str) {
      const char op = **str;
      *str += 2;
      const int right = add(linenum, str);
      switch (op) {
      case '<':   left <<= right; break;
      case '>':   left >>= right; break;
      default:
         fail(linenum, "invalid operator '%c'", op);
         return 0;
      }
   }
   return left;
}
static int cmp(size_t linenum, const char** str) {
   int left = shift(linenum, str);
   while (skip_ws(str), **str == '>' || **str == '<') {
      const bool gt = **str == '>';
      ++*str;
      const bool eq = **str == '=';
      if (eq) ++*str;
      const int right = shift(linenum, str);

      left = (gt ? left > right : left < right) || (eq && left == right);
   }
   return left;
}

static int equal(size_t linenum, const char** str) {
   int left = cmp(linenum, str);
   while (skip_ws(str), (**str == '!' || **str == '=') && (*str)[1] == '=') {
      const bool eq = **str == '=';
      *str += 2;
      const int right = cmp(linenum, str);
      left = !eq ^ (left == right);
   }
   return left;
}

static int bitand(size_t linenum, const char** str) {
   int left = equal(linenum, str);
   while (skip_ws(str), **str == '&' && (*str)[1] != **str) {
      ++*str;
      const int right = equal(linenum, str);
      left &= right;
   }
   return left;
}
static int bitor(size_t linenum, const char** str) {
   int left = bitand(linenum, str);
   while (skip_ws(str), **str == '|' && (*str)[1] != **str) {
      ++*str;
      const int right = bitand(linenum, str);
      left |= right;
   }
   return left;
}
static int bitxor(size_t linenum, const char** str) {
   int left = bitor(linenum, str);
   while (skip_ws(str), **str == '^') {
      ++*str;
      const int right = bitor(linenum, str);
      left ^= right;
   }
   return left;
}

static int booland(size_t linenum, const char** str) {
   int left = bitxor(linenum, str);
   while (skip_ws(str), **str == '&' && (*str)[1] == **str) {
      *str += 2;
      const int right = bitxor(linenum, str);
      left = left && right;
   }
   return left;
}
static int boolor(size_t linenum, const char** str) {
   int left = booland(linenum, str);
   while (skip_ws(str), **str == '|' && (*str)[1] == **str) {
      *str += 2;
      const int right = booland(linenum, str);
      left = left || right;
   }
   return left;
}

static int ternary(size_t linenum, const char** str) {
   const int cond = boolor(linenum, str);
   skip_ws(str);
   if (**str == '?') {
      ++*str;
      const int left = ternary(linenum, str);
      skip_ws(str);
      if (**str != ':') {
         fail(linenum, "expected ':' for ternary");
         return 0;
      }
      const int right = ternary(linenum, str);
      skip_ws(str);
      return cond ? left : right;
   } else return cond;
}

static int comma(size_t linenum, const char** str) {
   int val = ternary(linenum, str);
   while (skip_ws(str), **str == ',') {
      ++*str;
      val = ternary(linenum, str);
   }
   return val;
}

static int do_eval(size_t linenum, const char** str) {
   skip_ws(str);
   return comma(linenum, str);
}

int eval(size_t linenum, const char* str) {
   return do_eval(linenum, &str);
}
