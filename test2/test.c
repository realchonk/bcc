

int printf(const char*, ...);
const int a[] = { 1, 2, 3, 4, 5 };

int main(void) {
   for (unsigned i = 0; i < arraylen(a); ++i) {
      printf("%d\n", a[i]);
   }
}
