extern int printf(const char*, ...);

static int arr[10];

int main(void) {
   arr[0] = 42;
   return arr[0];
}

