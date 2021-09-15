#include <stddef.h>

extern void* __builtin_memcpy(void*, const void*, size_t);
extern void* memcpy(void*, const void*, size_t);
int printf(const char*, ...);

void* stub_memcpy(void* dest, const void* src, size_t num) {
   unsigned char* d = dest;
   const unsigned char* s = src;
   printf("dest=%p, src=%p, num=%zu\n", dest, src, num);
   printf("d=%p, s=%p\n", d, s);
   return dest;
}

int main(int argc, char* argv[]) {
   int a[] = { 1, 2, 3, 4, 5 };
   int b[5];
   //__builtin_memcpy(b, a, sizeof(a));
   stub_memcpy(b, a, sizeof(a));
   return b[0];
}
