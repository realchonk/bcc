#ifndef FILE_RISCV_CPU_H
#define FILE_RISCV_CPU_H
#include <stdbool.h>

struct riscv_cpu {
   bool is_embedded;
   bool has_mult;
   bool has_atomic;
   bool has_compressed;
   bool has_float;
   bool has_double;
   bool has_quad;
};

extern struct riscv_cpu riscv_cpu;

bool parse_cpu(const char*, struct riscv_cpu*);

#ifdef BCC_riscv32
#define BITS 32
#else
#define BITS 64
#endif

#endif /* FILE_RISCV_CPU_H */
