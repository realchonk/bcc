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
   buf_free(macro->params);
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
      free_macro(&e->macro);
      e->macro = *m;
   } else {
      e = malloc(sizeof(struct macro_entry));
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
   m.text = *arg ? arg + 1 : NULL;
   m.is_func = false;
   m.params = NULL;
   add_macro(&m);
}

// PREPROCESSOR DIRECTIVES

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
   m.is_func = false;
   m.text = "";
   m.params = NULL;
   size_t tki = 1;
   if (tki < num_tks) {
      if (tokens[tki].type == TK_WHITESPACE)
         ++tki;
      if (tki < num_tks)
         m.text = tokens[tki].begin;
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
