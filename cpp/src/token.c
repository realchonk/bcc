#include "token.h"

void print_token(FILE* file, const struct token* tk) {
   if (tk->type >= TOKEN_NAME) {

   } else {
      fputc(tk->ch, file);
   }
}
