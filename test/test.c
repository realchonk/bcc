int printf(const char*, ...);
void* malloc(unsigned long);
void free(int*);
void* memcpy(void*, const void*, unsigned);

int main(void) {
   char str[] = "Hello World";
   printf("str is '%s'\n", &str);
   return sizeof(int) * arraylen(str);
}


