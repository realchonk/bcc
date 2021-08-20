//  Copyright (C) 2021 Benjamin Stürz
//  
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//  
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <string.h>
#include "expand.h"
#include "token.h"
#include "dir.h"
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
static const struct var* find_var(const struct var* vars, istr_t name) {
   for (size_t i = 0; i < buf_len(vars); ++i) {
      if (name == vars[i].name)
         return &vars[i];
   }
   return NULL;
}

static const char* parse_string(const char* s) {
   char delim;
   if (*s == '"' || *s == '\'')
      delim = *s;
   else return NULL;
   ++s;
   while (*s) {
      if (*s == '\\') {
         ++s;
         if (*s == 'x') {
            ++s;
            for (int n = 0; n < 2 && isxdigit(*s); ++n, ++s);
         } else if (isodigit(*s)) {
            for (int n = 0; n < 3 && isodigit(*s); ++n, ++s);
         } else if (*s == 'u') {
            ++s;
            for (int n = 0; n < 4 && isxdigit(*s); ++n, ++s);
         } else if (*s == 'U') {
            ++s;
            for (int n = 0; n < 8 && isxdigit(*s); ++n, ++s);
         } else {
            ++s;
         }
      } else if (*s == delim)
         break;
      else ++s;
   }
   ++s;
   return s;
}

static const char* parse_param(const size_t linenum, const char* s) {
   size_t depth = 0;
   while (*s) {
      if (*s == '"' || *s == '\'') {
         s = parse_string(s);
         if (!s) {
            warn(linenum, "unterminated string");
         }
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

char* expand(size_t linenum, const char* s, struct var* vars, const char* macro_name, bool pre) {
   char* buf = NULL;
   while (*s) {
      if (*s == '"' || *s == '\'') {
         const char* begin = s;
         s = parse_string(s);
         if (!s) {
            warn(linenum, "unterminated string");
            return buf_free(buf), NULL;
         }
         while (begin != s) {
            buf_push(buf, *begin);
            ++begin;
         }
      } else if (*s == '#' && !pre) {
         ++s;
         if (*s == '#') {
            warn(linenum, "'##' cannot appear at either end of a macro expansion");
            return buf_free(buf), NULL;
         }
         const char* begin = s;
         while (isname(*s)) ++s;
         const istr_t name = strrint(begin, s);
         const struct var* v = find_var(vars, name);
         if (!v) {
            warn(linenum, "'#' is not followed by a macro parameter");
            return buf_free(buf), NULL;
         }
         char* e = expand(linenum, v->text, NULL, NULL, pre);
         if (!e) {
            warn(linenum, "failed to expand macro parameter");
            return buf_free(buf), NULL;
         }
         buf_insert(e, 0, '"');
         buf_last(e) = '"';
         buf_push(e, '\0');
         buf_puts(buf, e);
         buf_free(e);
      } else if (isname1(*s)) {
         const char* begin = s;
         while (isname(*s)) ++s;
         const istr_t name = strrint(begin, s);
         if (name == defined()) {
            while (isspace(*s)) ++s;
            const bool has_paren = *s == '(';
            if (has_paren) ++s;
            while (isspace(*s)) ++s;
            begin = s;
            while (isname(*s)) ++s;
            const istr_t name = strrint(begin, s);
            while (isspace(*s)) ++s;
            if (has_paren) {
               if (*s != ')') {
                  warn(linenum, "missing ')', got '%c'", *s);
                  return buf_free(buf), NULL;
               }
               ++s;
            }
            buf_puts(buf, "defined (");
            buf_puts(buf, name);
            buf_push(buf, ')');
            continue;
         }
         const struct var* v = find_var(vars, name);
         if (v) {
            char* e = expand(linenum, v->text, NULL, NULL, pre);
            if (!e) {
               warn(linenum, "failed to expand macro parameter");
               return buf_free(buf), NULL;
            }
            buf_puts(buf, e);
            buf_free(e);
            goto end_loop;
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
         if (m->type == MACRO_FUNC) {
            while (isspace(*s)) ++s;
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
                  char* e = expand(linenum, tmp, NULL, NULL, pre);
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
            char* e = expand_macro_func(m, params, pre);
            if (!e) return buf_free(buf), NULL;
            buf_puts(buf, e);
            for (size_t i = 0; i < buf_len(params); ++i)
               buf_free(params[i]);
            buf_free(params);
            buf_free(e);
         } else {
            char* e = expand_macro(linenum, m, pre);
            if (!e) return buf_free(buf), NULL;
            buf_puts(buf, e);
            buf_free(e);
         }
      } else {
         buf_push(buf, *s++);
      }
      
      if (*s == '#' && s[1] == '#') {
         s += 2;
      }

   end_loop:;
   }

   buf_push(buf, '\0');
   return buf;
}
char* expand_macro(size_t linenum, const struct macro* m, bool pre) {
   if (m->type == MACRO_SPEC) {
      char* s = m->handler(linenum);
      char* buf = NULL;
      buf_puts(buf, s);
      free(s);
      return buf;
   } else return expand(m->linenum, m->text, NULL, m->name, pre);
}
char* expand_macro_func(const struct macro* m, char** params, bool pre) {
   struct var* vars = NULL;
   for (size_t i = 0; i < buf_len(m->params); ++i) {
      struct var v;
      v.name = m->params[i];
      v.text = params[i];
      buf_push(vars, v);
   }
   char* e = expand(m->linenum, m->text, vars, m->name, pre);
   buf_free(vars);
   return e;
}
