#include "bcc.h"

unsigned popcnt(const uintmax_t val) {
   const unsigned len = sizeof(val) * 8;
   unsigned num = 0;
   for (unsigned i = 0; i < len; ++i) {
      num += (val >> i) & 1;
   }
   return num;
}
