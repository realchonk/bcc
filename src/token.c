#include <stdbool.h>
#include "strint.h"
#include "token.h"

const char* token_type_str[NUM_TOKENS] = {
   [TK_DUMMY]     = "dummy",
   [TK_INTEGER]   = "integer",
   [TK_FLOAT]     = "floating-point number",
   [TK_STRING]    = "string literal",
   [TK_CHARACTER] = "character literal",
   [TK_NAME]      = "identifier",
   
   [TK_LPAREN]    = "(",
   [TK_RPAREN]    = ")",
   [TK_LBRACK]    = "[",
   [TK_RBRACK]    = "]",
   [TK_CLPAREN]   = "{",
   [TK_CRPAREN]   = "}",
   [TK_PLUS]      = "+",
   [TK_MINUS]     = "-",
   [TK_STAR]      = "*",
   [TK_SLASH]     = "/",
   [TK_PLEQ]      = "+=",
   [TK_MIEQ]      = "-=",
   [TK_STEQ]      = "*=",
   [TK_SLEQ]      = "/=",
   [TK_PLPL]      = "++",
   [TK_MIMI]      = "--",
   [TK_NOT]       = "!",
   [TK_NEQ]       = "!=",
   [TK_EQ]        = "=",
   [TK_EQEQ]      = "==",
   [TK_GR]        = ">",
   [TK_GRGR]      = ">>",
   [TK_GREQ]      = ">=",
   [TK_GRGREQ]    = ">>=",
   [TK_LE]        = "<",
   [TK_LELE]      = "<<",
   [TK_LEEQ]      = "<=",
   [TK_LELEEQ]    = "<<=",
   [TK_COMMA]     = ",",
   [TK_COLON]     = ":",
   [TK_SEMICOLON] = ";",
   [TK_AMP]       = "&",
   [TK_AMPAMP]    = "&&",
   [TK_AMPEQ]     = "&=",
   [TK_PIPE]      = "|",
   [TK_PIPI]      = "||",
   [TK_PIPEEQ]    = "|=",
   [TK_XOR]       = "^",
   [TK_XOREQ]     = "^=",
   [TK_WAVE]      = "~",
   [TK_PERC]      = "%",
   [TK_PERCEQ]    = "%=",
   [TK_QMARK]     = "?",
   [TK_DDD]       = "...",
   [TK_DOT]       = ".",
   
   [TK_EOF]       = "end of file",

   [KW_CONST]     = "const",
   [KW_VOID]      = "void",
   [KW_SIGNED]    = "signed",
   [KW_UNSIGNED]  = "unsigned",
   [KW_BYTE]      = "byte",
   [KW_CHAR]      = "char",
   [KW_SHORT]     = "short",
   [KW_INT]       = "int",
   [KW_LONG]      = "long",
   [KW_FLOAT]     = "float",
   [KW_DOUBLE]    = "double",
   [KW_IF]        = "if",
   [KW_ELSE]      = "else",
   [KW_WHILE]     = "while",
   [KW_DO]        = "do",
   [KW_RETURN]    = "return",
   [KW_FOR]       = "for",
   [KW_BREAK]     = "break",
   [KW_CONTINUE]  = "continue",
   [KW_SIZEOF]    = "sizeof",
   [KW_ARRAYLEN]  = "arraylen",
};

void token_init(void) {
   static bool initialized = false;
   if (initialized) return;
   initialized = true;
   for (int i = TK_EOF + 1; i < NUM_TOKENS; ++i) {
      token_type_str[i] = strint(token_type_str[i]);
   }
}

void print_token(FILE* file, const struct token* tk) {
   switch (tk->type) {
   case TK_INTEGER:     fprintf(file, "%ju", tk->iVal); break;
   case TK_FLOAT:       fprintf(file, "%Lf", tk->fVal); break;
   case TK_STRING:      fprintf(file, "\"%s\"", tk->str); break;
   case TK_CHARACTER:   fprintf(file, "'%c'", tk->ch); break;
   case TK_NAME:        fputs(tk->str, file); break;
   default:             fputs(token_type_str[tk->type], file); break;
   }
}
void print_token_info(FILE* file, const struct token* tk) {
   fprintf(file, "token{ type: %s, begin: (", token_type_str[tk->type]);
   print_source_pos(file, &tk->begin);
   fputs("), end: (", file);
   print_source_pos(file, &tk->end);
   fputc(')', file);
   switch (tk->type) {
   case TK_INTEGER:     fprintf(file, ", value: %ju", tk->iVal); break;
   case TK_FLOAT:       fprintf(file, ", value: %Lf", tk->fVal); break;
   case TK_STRING:      fprintf(file, ", value: \"%s\"", tk->str); break;
   case TK_CHARACTER:   fprintf(file, ", value: '%c'", tk->ch); break;
   case TK_NAME:        fprintf(file, ", name: %s", tk->str); break;
   default:             break;
   }
   fputs(" }", file);
}
void print_source_pos(FILE* file, const struct source_pos* p) {
   fprintf(file, "%s:%zu:%zu", p->file, p->line + 1, p->column + 1);
}
