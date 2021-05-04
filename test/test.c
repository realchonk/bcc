int printf(const char*, ...);
void* malloc(unsigned long);
void free(int*);
void* memcpy(void*, const void*, unsigned);

short add(short a, short b) {
   return a + b;
}

int main(void) {
   char str[] = "Hello World";
   auto s = str;
   printf("s = %s\n", s);
   auto sum = add(3, 2);
   sum = 99;
   return sizeof(sum);
}


