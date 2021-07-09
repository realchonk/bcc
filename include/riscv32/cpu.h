#ifndef FILE_RISCV32_CPU_H
#define FILE_RISCV32_CPU_H
#include <stdbool.h>

struct riscv32_cpu {
   bool is_embedded;
   bool has_mult;
   bool has_atomic;
   bool has_compressed;
   bool has_float;
   bool has_double;
   bool has_quad;
};

extern struct riscv32_cpu riscv32_cpu;

bool parse_cpu(const char*, struct riscv32_cpu*);

#endif /* FILE_RISCV32_CPU_H */
