
int main(void) {
   int* a = (int*)(sizeof(int) * 12);
   int* b = (int*)(sizeof(int) * 10);
   return a - b;
}
