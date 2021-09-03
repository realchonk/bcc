#include <stddef.h>

#define f __divsi2

int printf(const char*, ...);
extern unsigned long f(unsigned long, unsigned long);


int main(int argc, char** argv) {
   int a = 11, b = 2;
   printf("f(%u, %u);\n", a, b);
}
