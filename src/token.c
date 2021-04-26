#include "token.h"

const char* token_type_str[NUM_TOKENS] = {
   [TK_DUMMY]     = "dummy",
   [TK_INTEGER]   = "integer",
   [TK_FLOAT]     = "float",
   [TK_STRING]    = "string",
   [TK_NAME]      = "name",
   
   [TK_EOF]       = "end of file"
};

