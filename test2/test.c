int printf(const char*, ...);
int atoi(const char*);

/*
int fib(int n) {
   if (n < 2) {
      return n;
   } else {
      const int a = fib(n - 1);
      const int b = fib(n - 2);
      return a + b;
   }
}*/

int fib2(int n) {
   if (n < 2) {
      return n;
   } else {
      return fib2(n - 1) + fib2(n - 2);
   }
}

int main(int argc, char** argv) {
   if (argc != 2) {
      printf("Usage: %s <number>\n", argv[0]);
      return 1;
   }
   const int a = atoi(argv[1]);
   printf("fib(%d) = %d\n", a, fib2(a));
}
