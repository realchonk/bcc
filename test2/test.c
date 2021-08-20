int printf(const char*, ...);
int putchar(int);

int main(void) {
   int arr[8];
   for (unsigned i = 0; i < arraylen(arr); ++i) {
      arr[i] = i;
   }
   for (unsigned i = 0; i < arraylen(arr); ++i) {
      printf("arr[%u]=%u\n", i, arr[i]);
   }
   return 0;
}
