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

#include "macro.h"
#include "dir.h"
#include "cpp.h"
#include "if.h"

bool suppress_code = false;

struct if_layer* if_layers = NULL;

static void update_suppress(void) {
   suppress_code = false;
   for (size_t i = 0; i < buf_len(if_layers); ++i) {
      const struct if_layer* l = &if_layers[i];
      if (l->type == LAY_ELIF) {
         if (l->prev || !l->value) {
            suppress_code = true;
            return;
         }
      } else if (!l->value) {
         suppress_code = true;
         return;
      }
   }
}

static bool ifdef_impl(size_t linenum, struct token* tokens, size_t num_tks, bool negate) {
   if (num_tks < 1) {
      warn(linenum, "expected macro name");
      return false;
   }
   if (tokens[0].type != TK_WORD) {
      warn(linenum, "expected macro name, got '%s'", token_type_str[tokens[0].type]);
      return false;
   }

   istr_t name = strrint(tokens[0].begin, tokens[0].end);
   struct if_layer if_layer;
   if_layer.type = LAY_IF;
   if_layer.value = negate ^ (get_macro(name) != NULL);
   buf_push(if_layers, if_layer);
   update_suppress();
   return true;

}

bool dir_ifdef(size_t linenum, const char* line, struct token* tokens, size_t num_tks, FILE* out) {
   (void)out;
   (void)line;
   return ifdef_impl(linenum, tokens, num_tks, false);
}
bool dir_ifndef(size_t linenum, const char* line, struct token* tokens, size_t num_tks, FILE* out) {
   (void)out;
   (void)line;
   return ifdef_impl(linenum, tokens, num_tks, true);
}
bool dir_endif(size_t linenum, const char* line, struct token* tokens, size_t num_tks, FILE* out) {
   (void)line;
   (void)tokens;
   (void)num_tks;
   (void)out;
   if (buf_len(if_layers) < 1) {
      warn(linenum, "invalid #endif");
      return false;
   }
   buf_pop(if_layers);
   update_suppress();
   return true;
}
bool dir_else(size_t linenum, const char* line, struct token* tokens, size_t num_tks, FILE* out) {
   (void)line;
   (void)tokens;
   (void)num_tks;
   (void)out;
   if (buf_len(if_layers) < 1) {
      warn(linenum, "invalid #else");
      return false;
   }
   struct if_layer* if_layer = &buf_last(if_layers);
   switch (if_layer->type) {
   case LAY_IF:
      if_layer->value = !if_layer->value;
      break;
   case LAY_ELIF:
      if_layer->value = !if_layer->prev && !if_layer->value;
      break;
   default:
      warn(linenum, "#else has to come after #if or #elif or #elifdef or #elifndef");
      return false;
   }
   if_layer->type = LAY_ELSE;
   update_suppress();
   return true;
}

bool dir_if(size_t linenum, const char* line, struct token* tokens, size_t num_tks, FILE* out) {
   (void)line;
   (void)out;
   if (num_tks < 1) {
      warn(linenum, "expected expression");
      return false;
   }
   struct if_layer if_layer;
   if_layer.type = LAY_IF;
   if_layer.value = eval(linenum, tokens[0].begin);
   buf_push(if_layers, if_layer);
   update_suppress();
   return true;
}
bool dir_elif(size_t linenum, const char* line, struct token* tokens, size_t num_tks, FILE* out) {
   (void)line;
   (void)out;
   if (buf_len(if_layers) < 1) {
      warn(linenum, "invalid #elif");
      return false;
   }
   if (num_tks < 1) {
      warn(linenum, "expected expression");
      return false;
   }
   struct if_layer* if_layer = &buf_last(if_layers);
   switch (if_layer->type) {
   case LAY_IF:
      if_layer->prev = if_layer->value;
      break;
   case LAY_ELIF:
      if_layer->prev |= if_layer->value;
      break;
   default:
      warn(linenum, "#elif has to come after #if or #elif");
      return false;
   }
   if_layer->type = LAY_ELIF;
   if_layer->value = eval(linenum, tokens[0].begin);
   update_suppress();
   return true;
}

static bool elifdef_impl(size_t linenum, struct token* tokens, size_t num_tks, bool negate) {
   if (buf_len(if_layers) < 1) {
      warn(linenum, "invalid #elifdef");
      return false;
   }
   if (num_tks < 1) {
      warn(linenum, "expected macro name");
      return false;
   }

   istr_t name = strrint(tokens[0].begin, tokens[0].end);
   struct if_layer* if_layer = &buf_last(if_layers);
   switch (if_layer->type) {
   case LAY_IF:
      if_layer->prev = if_layer->value;
      break;
   case LAY_ELIF:
      if_layer->prev |= if_layer->value;
      break;
   default:
      warn(linenum, "#elifdef has to come after #if or #elif or #elifdef or #elifndef");
      return false;
   }
   if_layer->type = LAY_ELIF;
   if_layer->value = negate ^ (get_macro(name) != NULL);
   update_suppress();
   return true;
}

bool dir_elifdef(size_t linenum, const char* line, struct token* tokens, size_t num_tks, FILE* out) {
   (void)line;
   (void)out;
   return elifdef_impl(linenum, tokens, num_tks, false);
}

bool dir_elifndef(size_t linenum, const char* line, struct token* tokens, size_t num_tks, FILE* out) {
   (void)line;
   (void)out;
   return elifdef_impl(linenum, tokens, num_tks, true);
}
