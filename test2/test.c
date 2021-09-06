#include <stddef.h>

#define f __divsi2

int printf(const char*, ...);
extern unsigned long f(unsigned long, unsigned long);

struct A {
   int a;
   int b;
};

int main(int argc, char** argv) {
   int a = 10, b = 9;
   return a % b;
}
