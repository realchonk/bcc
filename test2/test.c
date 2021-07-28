
int printf(const char*, ...);

int f(void) { return 10; }
int g(void) { return 20; }


int main(void) {
   const int r = f() + g();
   printf("%d\n", r);
}
