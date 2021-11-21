
int printf(const char* fmt, ...);

static void puti(int i) {
   printf("%d\n", i);
}

int main(int argc) {
   puti(42);
}
