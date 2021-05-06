extern int printf(const char*, ...);

typedef unsigned long size_t;

static int arr[10];

int main(void) {
   const size_t i = arraylen(arr);

   return i;
}

