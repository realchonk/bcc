
int add(int a, int b) {
   return a + b;
}

int main(void) {
   auto f = &add;
   return f(3, 2);
}
