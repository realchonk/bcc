#include <stddef.h>

#define f __divsi2

int printf(const char*, ...);
extern unsigned long f(unsigned long, unsigned long);


int main(void) {
   unsigned long a = 11, b = 2;
   unsigned long r = a / b + a % b;
   printf("f(%ld, %ld) = %ld\n", a, b, r);
}
