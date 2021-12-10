
int printf(const char* fmt, ...);

static void puti(int i) {
   printf("%d\n", i);
}

int main(int argc) {
   const int x = 42;
   *(int*)&x = 69;
   return x;
}
