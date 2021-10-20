

int printf(const char*, ...);

int global_var = 1;

int add(int a, int b) {
   int result = a + b;
   ++global_var;
   return result;
}

int f(int x) {
   return x;
}


int main(void) {
   int a = 20;
   int b = 5;
   return a + b;
}
