#ifndef FILE_BCC_H
#define FILE_BCC_H
#include <stdbool.h>
#include <stdint.h>

extern bool enable_warnings;
extern unsigned optim_level;
extern bool console_colors;

unsigned popcnt(uintmax_t);
#define is_pow2(n) (popcnt(n) == 1)

#endif /* FILE_BCC_H */

