#ifndef FILE_RISCV32_REGS_H
#define FILE_RISCV32_REGS_H
#include "riscv/cpu.h"
#include "error.h"

#if BITS == 32
typedef uint32_t uintreg_t;
typedef int32_t intreg_t;
#define REGSIZE 4
#define LW "lw"
#define SW "sw"
#define DEF_MACH "rv32gc"
#else
typedef uint64_t uintreg_t;
typedef int64_t intreg_t;
#define REGSIZE 8
#define LW "ld"
#define SW "sd"
#define DEF_MACH "rv64gc"
#endif


extern const char* regs[15];

#define reg_op(r) ((r) < arraylen(regs) ? regs[r] : (panic("register out of range"), NULL))


#define align_stack_size(sz) ((uintreg_t)(((uintreg_t)(sz) & 15) ? (((uintreg_t)(sz) & ~15) + 16) : (uintreg_t)(sz)))

#endif /* FILE_RISCV32_REGS_H */
