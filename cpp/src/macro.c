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

#include "expand.h"
#include "token.h"
#include "macro.h"
#include "dir.h"
#include "cpp.h"

struct macro_entry {
   struct macro_entry* next;
   struct macro_entry* prev;
   struct macro macro;
};

static struct macro_entry* macros = NULL;

static void free_macro(struct macro* macro) {
   switch (macro->type) {
   case MACRO_FUNC:
      buf_free(macro->params);
      break;
   }
}

static struct macro_entry* find_me(istr_t name) {
   for (struct macro_entry* e = macros; e; e = e->next) {
      if (name == e->macro.name)
         return e;
   }
   return NULL;
}


void add_macro(const struct macro* m) {
   struct macro_entry* e = find_me(m->name);
   if (e) {
      if (e->macro.type == MACRO_SPEC) {
         fail(m->linenum, "redefining special macro '%s' is not allowed", e->macro.name);
         return;
      }
      free_macro(&e->macro);
      e->macro = *m;
   } else {
      e = malloc(sizeof(struct macro_entry));
      if (!e)
         panic("failed to allocate macro");
      e->next = macros;
      e->prev = NULL;
      e->macro = *m;
      if (macros) macros->prev = e;
      macros = e;
   }
}

bool remove_macro(istr_t name) {
   struct macro_entry* e = find_me(name);
   
   if (e) {
      if (e->macro.type == MACRO_SPEC) {
         fail(1, "undefining special macro '%s' is not allowed", e->macro.name);
         return false;
      }
      if (e->prev) e->prev->next = e->next;
      else macros = e->next;
      if (e->next) e->next->prev = e->prev;
      free_macro(&e->macro);
      free(e);
   }

   return e != NULL;
}
const struct macro* get_macro(istr_t name) {
   const struct macro_entry* e = find_me(name);
   return e ? &e->macro : NULL;
}
void add_cmdline_macro(const char* arg) {
   struct macro m;
   const char* begin_name = arg;
   while (*arg && *arg != '=') {
      ++arg;
   }

   m.name = strrint(begin_name, arg);
   m.type = MACRO_VAR;
   m.text = *arg ? arg + 1 : "1";
   add_macro(&m);
}

// PREPROCESSOR DIRECTIVES

#define skip_ws() if (tki < num_tks && tokens[tki].type == TK_WHITESPACE) ++tki

bool dir_define(size_t linenum, const char* line, struct token* tokens, size_t num_tks, FILE* out) {
   (void)line;
   (void)out;
   if (num_tks < 1) {
      warn(linenum, "expected word");
      return false;
   } else if (tokens[0].type != TK_WORD) {
      warn(linenum, "expected word, got %s", token_type_str[tokens[0].type]);
      return false;
   }
   struct macro m;
   m.name = strnint(tokens[0].begin, tokens[0].end - tokens[0].begin);
   m.type = MACRO_VAR;
   m.text = "";
   m.linenum = linenum;
   size_t tki = 1;
   if (tki < num_tks) {
      const char* s = tokens[tki].begin;
      if (*s == '(') {
         m.type = MACRO_FUNC;
         m.params = NULL;
         ++s;
         while (isspace(*s)) ++s;
         if (*s != ')') {
            do {
               while (isspace(*s)) ++s;
               const char* begin = s;
               if (!isname1(*s)) {
                  warn(linenum, "expected name for parameter");
                  return false;
               }
               ++s;
               while (isname(*s)) ++s;
               const istr_t name = strrint(begin, s);
               buf_push(m.params, name);
               while (isspace(*s)) ++s;
            } while (*s++ == ',');
            if (s[-1] != ')') {
               warn(linenum, "missing ')'");
               return false;
            }
            while (isspace(*s)) ++s;
            m.text = expand(linenum, s, NULL, m.name, true);
            if (!m.text) {
               warn(linenum, "failed to expand macro function");
               return false;
            }
         }
      } else {
         while (isspace(*s)) ++s;
         m.text = expand(linenum, s, NULL, m.name, false);
         if (!m.text) {
            warn(linenum, "failed to expand macro");
            return false;
         }
      }
   }
   add_macro(&m);
   return true;
}
bool dir_undef(size_t linenum, const char* line, struct token* tokens, size_t num_tks, FILE* out) {
   (void)line;
   (void)out;
   if (num_tks < 1) {
      warn(linenum, "expected word");
      return false;
   } else if (tokens[0].type != TK_WORD) {
      warn(linenum, "expected word, got %s", token_type_str[tokens[0].type]);
      return false;
   }
   remove_macro(strnint(tokens[0].begin, tokens[0].end - tokens[0].begin));
   return true;
}
void dump_macros(FILE* out) {
   for (const struct macro_entry* e = macros; e; e = e->next) {
      const struct macro* m = &e->macro;
      switch (m->type) {
      case MACRO_VAR:
         fprintf(out, "#define %s %s\n", m->name, m->text ? m->text : "");
         break;
      case MACRO_FUNC:
         fprintf(out, "#define %s(", m->name);
         if (buf_len(m->params)) {
            fputs(m->params[0], out);
            for (size_t i = 1; i < buf_len(m->params); ++i) {
               fprintf(out, ",%s", m->params[i]);
            }
         }
         fprintf(out, ") %s\n", m->text);
         break;
      }
   }
}
