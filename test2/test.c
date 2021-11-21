
int get() {
   return 4;
}

int main(int argc) {
   auto f = &get;
   return f(4, 2, 1);
}
