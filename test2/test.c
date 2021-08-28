#include <stddef.h>

struct A {
   int a;
   int b;
};

int main() {
   struct A a;
   return sizeof(a);
}
