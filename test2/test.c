int printf(const char*, ...);
int putchar(int);

inline int add(int x, int y) {
   return x + y;
}
extern inline int add(int, int);

int main(void) {
   return add(3, 2);
}
