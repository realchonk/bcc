int printf(const char*, ...);

_Bool toBool(int x) { return x; }
int toInt(_Bool x) { return x; }
int main(void) {
   _Bool b = toBool(42);
   int i = toInt(b);
   return b + i;
}
