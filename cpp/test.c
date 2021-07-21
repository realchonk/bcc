#include "cpp.h"

int printf(const char*, ...);
int main(void) {
   int a = 42;
   int b = 99;
   printf("%d", (a * b) + (b / a));
}

