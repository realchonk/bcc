int printf(const char*, ...);
void* malloc(unsigned long);
void free(int*);
void* memcpy(void*, const void*, unsigned);

int main(void) {
   char str[] = "Hello World";
   auto s = str;
   printf("s = %s\n", s);
   return sizeof(s);
}


