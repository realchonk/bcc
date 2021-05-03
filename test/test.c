int printf(const char* fmt, ...);
void* malloc(unsigned long sz);
void free(int* a);

int main(void) {
   int* a = (int*)malloc(16 * 4);
   for (unsigned i = 0; i < 16; ++i) {
      a[i] = i;
   }
   for (unsigned i = 0; i < 16; ++i) {
      printf("a[%u] = %d\n", i, a[i]);
   }
   free(a);
   return a[15];
}


