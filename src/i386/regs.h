#include "error.h"

typedef uint32_t uintreg_t;
static const char* regs8[] =  {  "al",  "ch",  "dl" };
static const char* regs16[] = {  "ax",  "cx",  "dx" };
static const char* regs32[] = { "eax", "ecx", "edx" };

#define REGSIZE 4
#define reg_sp "esp"
#define reg_bp "ebp"
#define reg_ax "eax"
#define reg_bx "ebx"
#define reg_dx "edx"
#define reg_dxi 2

#define reg8(i) ((i) < arraylen(regs8) ? regs8[i] : (panic("register out of range"), NULL))
#define reg16(i) ((i) < arraylen(regs16) ? regs16[i] : (panic("register out of range"), NULL))
#define reg32(i) ((i) < arraylen(regs32) ? regs32[i] : (panic("register out of range"), NULL))
#define mreg(i) reg32(i)

#define reg_op(dest, src, size) \
   switch (size) { \
   case IRS_BYTE: \
   case IRS_CHAR:    dest = reg8(src); break; \
   case IRS_SHORT:   dest = reg16(src); break; \
   case IRS_PTR: \
   case IRS_INT:     dest = reg32(src); break; \
   default:          panic("unsupported operand size '%s'", ir_size_str[size]); \
   }

