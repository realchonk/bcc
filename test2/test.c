

int printf(const char*, ...);

// int add(int a, int b) {
//    return a + b;
// }

int fib(int n) {
   printf("fib(%d);\n", n);
   if (n <= 1) {
      return n;
   }
   const int a = fib(n - 1);
   const int b = fib(n - 2);
   return a + b;
}

//int radd(int a, int b) {
//   printf("radd(%d, %d);\n", a, b);
//   if (b == 0) {
//      return a;
//   } else {
//      return radd(a + 1, b - 1);
//   }
//}

int main(void) {
   return fib(10);
}
