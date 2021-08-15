#include <string.h>
#include "expand.h"
#include "token.h"
#include "cpp.h"

// buf.h helper functions
static char* buf_puts_impl(char* buf, const char* s) {
   while (*s) {
      buf_push(buf, *s++);
   }
   return buf;
}
static char* buf_putsr_impl(char* buf, const char* begin, const char* end) {
   while (begin != end) {
      buf_push(buf, *begin++);
   }
   return buf;
}
#define buf_putsr(buf, begin, end) ((buf) = (buf_puts_impl((buf), (begin), (end))))
#define buf_puts(buf, s) ((buf) = (buf_puts_impl((buf), (s))))

// other helper functions

static const char* parse_param(const size_t linenum, const char* s) {
   size_t depth = 0;
   while (*s) {
      if (*s == '"') {
         ++s;
         while (*s) {
            if (*s == '"' && s[-1] != '\\')
               break;
            else ++s;
         }
         ++s;
      } else if (*s == '\'') {
         ++s;
         while (*s) {
            if (*s == '\'' && s[-1] != '\\')
               break;
            else ++s;
         }
         ++s;
      } else if (*s == '(') {
         ++depth;
         ++s;
      } else if (*s == ')') {
         if (!depth) break;
         else --depth, ++s;
      } else if (*s == ',') {
         if (!depth) break;
         else ++s;
      } else ++s;
   }
   if (depth) {
      warn(linenum, "unterminated argument list");
      return NULL;
   }
   return s;
}

// actual expansion

char* expand2(size_t linenum, const char* s, struct var* vars, const char* macro_name) {
   const size_t nv = buf_len(vars);
   char* buf = NULL;
   while (*s) {
      if (*s == '"') {
         ++s;
         buf_push(buf, '"');
         while (1) {
            if (!*s) {
               warn(linenum, "unterminated string");
               return buf_free(buf), NULL;
            }
            if (*s == '"' && s[-1] != '\\')
               break;
            buf_push(buf, *s);
            ++s;
         }
         ++s;
         buf_push(buf, '"');
      } else if (*s == '\'') {
         ++s;
         buf_push(buf, '\'');
         while (1) {
            if (*s == '\'' && s[-1] != '\\')
               break;
            buf_push(buf, *s);
            ++s;
         }
         ++s;
         buf_push(buf, '\'');
      } else if (isname1(*s)) {
         const char* begin = s;
         while (isname(*s)) ++s;
         const istr_t name = strrint(begin, s);
         for (size_t i = 0; i < nv; ++i) {
            if (vars[i].name == name) {
               char* e = expand2(linenum, vars[i].text, NULL, macro_name);
               if (!e) {
                  warn(linenum, "failed to expand");
                  return buf_free(buf), NULL;
               }
               buf_puts(buf, e);
               buf_free(e);
               goto end_loop;
            }
         }
         if (name == macro_name) {
            buf_puts(buf, name);
            continue;
         }
         const struct macro* m = get_macro(name);
         if (!m) {
            buf_puts(buf, name);
            continue;
         }
         if (m->is_func) {
            if (*s != '(') {
               warn(linenum, "expected '(' for expansion of '%s'", name);
               return buf_free(buf), NULL;
            }
            ++s;
            while (isspace(*s)) ++s;
            char** params = NULL;
            if (*s != ')') {
               while (1) {
                  const char* begin = s;
                  s = parse_param(linenum, s);
                  char* tmp = strndup(begin, s - begin);
                  char* e = expand2(linenum, tmp, NULL, NULL);
                  free(tmp);
                  if (!e) {
                     warn(linenum, "failed to expand");
                     return buf_free(buf), NULL;
                  }
                  buf_push(params, e);
                  if (*s == ')') break;
                  else if (*s == ',') ++s;
                  else {
                     warn(linenum, "expected ')' or ',', got '%c'", *s);
                     return buf_free(buf), NULL;
                  }
               }
               ++s;
            }
            if (buf_len(m->params) != buf_len(params)) {
               warn(linenum, "macro '%s' requires %zu arguments, but only %zu given", m->name,
                     buf_len(m->params), buf_len(params));
               return buf_free(buf), NULL;
            }
            char* e = expand_macro_func(m, params);
            if (!e) return buf_free(buf), NULL;
            buf_puts(buf, e);
            for (size_t i = 0; i < buf_len(params); ++i)
               buf_free(params[i]);
            buf_free(params);
            buf_free(e);
         } else {
            char* e = expand_macro(m);
            if (!e) return buf_free(buf), NULL;
            buf_puts(buf, e);
            buf_free(e);
         }
      } else {
         buf_push(buf, *s++);
      }
   end_loop:;
   }

   buf_push(buf, '\0');
   return buf;
}
char* expand_macro(const struct macro* m) {
   return expand2(m->linenum, m->text, NULL, m->name);
}
char* expand_macro_func(const struct macro* m, char** params) {
   struct var* vars = NULL;
   for (size_t i = 0; i < buf_len(m->params); ++i) {
      struct var v;
      v.name = m->params[i];
      v.text = params[i];
      buf_push(vars, v);
   }
   char* e = expand2(m->linenum, m->text, vars, m->name);
   buf_free(vars);
   return e;
}
char* expand(size_t linenum, const char* begin, const char* end) {
   char* s = strndup(begin, end - begin);
   char* e = expand2(linenum, s, NULL, NULL);
   free(s);
   return e;
}
