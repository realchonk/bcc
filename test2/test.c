#include <stddef.h>

extern void* __builtin_memcpy(void*, const void*, size_t);
extern void* memcpy(void*, const void*, size_t);
int printf(const char*, ...);

int print_ints(int a, int b) {
   //printf("%d, %d\n", a, b);
   //:wq
   return a + b;
}

int main(int argc, char* argv[]) {
   int a = 32, b = 16;
   return print_ints(a, b);
}
