int printf(const char*, ...);

int add(int a, int b) {
   return a + b;
}

int main(void) {
   int a = 42;
   int b = 24;
   printf("Hello World a=%d b=%d (a + b)=%d\n", a, b, a + b);
}
