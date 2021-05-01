#include "error.h"

typedef uint64_t uintreg_t;
static const char* regs8[] =  {  "al", "dil", "sil",  "dl",  "cl", "r8b", "r9b", "r10b", "r11b" };
static const char* regs16[] = {  "ax",  "di",  "si",  "dx",  "cx", "r8w", "r9w", "r10w", "r11w" };
static const char* regs32[] = { "eax", "edi", "esi", "edx", "ecx", "r8d"  "r9d", "r10d", "r11d" };
static const char* regs64[] = { "rax", "rdi", "rsi", "rdx", "rcx",  "r8",  "r9",  "r10",  "r11" };
static size_t param_regs[] = { 1, 2, 3, 4, 5, 6 };

#define REGSIZE 8
#define reg_sp "rsp"
#define reg_bp "rbp"
#define reg_ax "rax"
#define reg_dx "rdx"

#define reg8(i) ((i) < arraylen(regs8) ? regs8[i] : (panic("emit_ir(): register out of range"), NULL))
#define reg16(i) ((i) < arraylen(regs16) ? regs16[i] : (panic("emit_ir(): register out of range"), NULL))
#define reg32(i) ((i) < arraylen(regs32) ? regs32[i] : (panic("emit_ir(): register out of range"), NULL))
#define reg64(i) ((i) < arraylen(regs64) ? regs64[i] : (panic("emit_ir(): register out of range"), NULL))
#define mreg(i) reg64(i)

#define reg_op(dest, src, size) \
   switch (size) { \
   case IRS_BYTE: \
   case IRS_CHAR:    dest = reg8(src); break; \
   case IRS_SHORT:   dest = reg16(src); break; \
   case IRS_INT:     dest = reg32(src); break; \
   case IRS_PTR: \
   case IRS_LONG:    dest = reg64(src); break; \
   default:          panic("emit_ir(): unsupported operand size '%s'", ir_size_str[size]); \
   }

