int printf(const char*, ...);

int add(int a, int b) {
   int sum = a + b;
   return sum;
}

int main(void) {
   int a = 42, b = 4;
   printf("a=%d\nb=%d\na+b=%d\n", a, b, add(a, b));
}
