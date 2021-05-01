

int main(int argc, char** argv) {
   for (int i = 0; i < 10; ++i) {
      if (i == 5) continue;
      else if (i == 7) break;
      printf("%d\n", i);
   }
}
