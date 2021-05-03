int printf(const char* fmt, ...);
void* malloc(unsigned long sz);
void free(int* a);

int main(void) {
   int a[0x10];
   for (unsigned i = 0; i < 16; ++i) {
      a[i] = i;
   }
   for (unsigned i = 0; i < 16; ++i) {
      printf("a[%u] = %d\n", i, a[i]);
   }
   return a[15];
}


