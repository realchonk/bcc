int printf(const char*, ...);

int atoi(const char*);

int main(int argc, char** argv) {
   if (argc < 3) {
      printf("Usage: %s <integer> <integer>\n", argv[0]);
      return 1;
   }
   const int a = atoi(argv[1]);
   const int b = atoi(argv[2]);
   printf("%d * %d = %d\n", a, b, a * b);
   printf("%d / %d = %d\n", a, b, a / b);
   printf("%d %% %d = %d\n", a, b, a % b);
}
