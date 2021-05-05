int printf(const char*, ...);
void* malloc(unsigned long);
void free(int*);
void* memcpy(void*, const void*, unsigned);
int putchar(int);

unsigned get() { return 10; }

int main() {
   unsigned arr[get()];

   for (unsigned i = 0; i < arraylen(arr); ++i) {
      arr[i] = i;
   }

   for (unsigned i = arraylen(arr); i != 0; --i) {
      printf("arr[%d]=%d\n", i - 1, arr[i - 1]);
   }
   
   return arraylen(arr);
}


