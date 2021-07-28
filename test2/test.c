int printf(const char*, ...);
int atoi(const char*);

void f(void) {
   int arr[8];
   for (unsigned i = 0; i < arraylen(arr); ++i) {
      arr[i] = i;
   }
   for (unsigned i = 0; i < arraylen(arr); ++i) {
      printf("arr[%u] = %u\n", i, arr[i]);
   }
   printf("finished\n");
}
int main(void) {
   f();
   printf("f finished\n");
   return 0;
}
