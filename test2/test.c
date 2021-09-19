

int printf(const char*, ...);

int add(int a, int b) {
   return a + b;
}

struct S {
   int a;
};

int main(void) {
   struct S a;
   a.a = 42;
   struct S b = a;
   return 1 + (b.a + (2 * add(4, add(1, 4 * add(4, 1)))));
}
