int printf(const char*, ...);
void* malloc(unsigned long);
void free(int*);
void* memcpy(void*, const void*, unsigned);

short get() {
   int s = -1;
   return s;
}

int main() {
   printf("%d\n", get());
}


