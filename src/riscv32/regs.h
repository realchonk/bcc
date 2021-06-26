#include "error.h"

typedef uint32_t uintreg_t;
static const char* regs[] = { "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "t0", "t1", "t2", "t3", "t4", "t5", "t6" };

#define reg_op(r) ((r) < arraylen(regs) ? regs[r] : (panic("register out of range"), NULL))

#define REGSIZE 4
