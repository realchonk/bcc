#include <stddef.h>

int puts(const char*);

int main(void) {
   char str[] = "Hello World";
   puts(str);
   return 0;
}
