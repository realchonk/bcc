int printf(const char*, ...);

struct S {
   int x;
};

struct S get_s(void) {
   struct S s;
   s.x = 42;
   return s;
}

int main(void) {
   struct S s;
   s = get_s();
   return s.x;
}
