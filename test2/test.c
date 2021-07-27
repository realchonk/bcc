int printf(const char*, ...);

union A {
   int a;
   int b;
};

int main(void) {
   union A a;
   return &a.a == &a.b;
}
