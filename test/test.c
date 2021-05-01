

int main(int argc, char** argv) {
   printf("argc = %d\n", argc);
   for (int i = 1; i < argc; ++i) {
      char* s = *(argv + (i * 4));
      printf("argv[%d]=%s\n", i, s);
   }
   return 0;
}
