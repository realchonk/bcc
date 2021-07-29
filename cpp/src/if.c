#include "macro.h"
#include "dir.h"
#include "cpp.h"

bool suppress_code = false;

enum layer_type {
   LAY_IF,
   LAY_ELIF,
   LAY_ELSE,
};

struct layer {
   enum layer_type type;
   bool value;

   bool prev; // for elif
};

static struct layer* layers = NULL;

static void update_suppress(void) {
   suppress_code = false;
   for (size_t i = 0; i < buf_len(layers); ++i) {
      const struct layer* l = &layers[i];
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
   struct layer layer;
   layer.type = LAY_IF;
   layer.value = !negate && (get_macro(name) != NULL);
   buf_push(layers, layer);
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
   if (buf_len(layers) < 1) {
      warn(linenum, "invalid #endif");
      return false;
   }
   buf_pop(layers);
   update_suppress();
   return true;
}
bool dir_else(size_t linenum, const char* line, struct token* tokens, size_t num_tks, FILE* out) {
   (void)line;
   (void)tokens;
   (void)num_tks;
   (void)out;
   if (buf_len(layers) < 1) {
      warn(linenum, "invalid #else");
      return false;
   }
   struct layer* layer = &buf_last(layers);
   switch (layer->type) {
   case LAY_IF:
      layer->value = !layer->value;
      break;
   case LAY_ELIF:
      layer->value = !layer->prev && !layer->value;
      break;
   default:
      warn(linenum, "#else has to come after #if or #elif");
      return false;
   }
   layer->type = LAY_ELSE;
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
   struct layer layer;
   layer.type = LAY_IF;
   layer.value = eval(linenum, tokens[0].begin);
   buf_push(layers, layer);
   update_suppress();
   return true;
}
bool dir_elif(size_t linenum, const char* line, struct token* tokens, size_t num_tks, FILE* out) {
   (void)line;
   (void)out;
   if (buf_len(layers) < 1) {
      warn(linenum, "invalid #elif");
      return false;
   }
   if (num_tks < 1) {
      warn(linenum, "expected expression");
      return false;
   }
   struct layer* layer = &buf_last(layers);
   switch (layer->type) {
   case LAY_IF:
      layer->prev = layer->value;
      break;
   case LAY_ELIF:
      layer->prev |= layer->value;
      break;
   default:
      warn(linenum, "#elif has to come after #if or #elif");
      return false;
   }
   layer->type = LAY_ELIF;
   layer->value = eval(linenum, tokens[0].begin);
   update_suppress();
   return true;
}
