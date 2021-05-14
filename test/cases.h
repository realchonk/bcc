{
   .name = "simple printf",
   .compiles = true,
   .source =
      "int printf(const char*, ...);"
      "int main(void) {"
      "  printf(\"Hello World\\n\");"
      "}",
   .output = "Hello World\n",
},
{
   .name = "sizeof(array)",
   .compiles = true,
   .source =
      "int main(void) {"
      "  int arr[10];"
      "  return sizeof(arr);"
      "}",
   .ret_val = 40,
},
{
   .name = "arraylen(VLA)",
   .compiles = true,
   .source =
      "int printf(const char*, ...);"
      "unsigned get(void) { return 10; }"
      "int main(void) {"
      "  int arr[get()];"
      "  printf(\"%u\", arraylen(arr));"
      "}",
   .output = "10",
},
{
   .name = "typedef",
   .compiles = true,
   .source =
      "typedef unsigned byte uint_t;"
      "int main(void) {"
      "  return sizeof(uint_t);"
      "}",
   .ret_val = 1,
},
{
   .name = "for-array",
   .compiles = true,
   .source =
      "int printf(const char*, ...);"
      "int main(void) {\n"
      "  int arr[8];"
      "  for (unsigned i = 0; i < arraylen(arr); ++i) {"
      "     arr[i] = i;"
      "  }"
      "  for (unsigned i = 0; i < arraylen(arr); ++i) {"
      "     printf(\"arr[%u]=%u\\n\", i, arr[i]);"
      "  }"
      "  return 42;"
      "}",
   .output =
      "arr[0]=0\n"
      "arr[1]=1\n"
      "arr[2]=2\n"
      "arr[3]=3\n"
      "arr[4]=4\n"
      "arr[5]=5\n"
      "arr[6]=6\n"
      "arr[7]=7\n",
   .ret_val = 42,
},
{
   .name = "multiply/divide",
   .compiles = true,
   .source =
      "int printf(const char*, ...);"
      "int main(void) {"
      "  int a = 42;"
      "  int b = 99;"
      "  printf(\"%d\", (a * b) + (b / a));"
      "}",
   .output = "4160",
   .ret_val = 0
},
{
   .name = "casting of integers",
   .compiles = true,
   .source =
      "int printf(const char*, ...);"
      "int main(void) {"
      "  int a = -1;"
      "  printf(\"%d\", (short)a);"
      "}",
   .output = "-1",
   .ret_val = 0
},
{
   .name = "nested for-loop",
   .compiles = true,
   .source =
      "int putchar(int);"
      "int main(void) {"
      "  for (int a = 0; a < 3; ++a) {"
      "     for (int b = 0; b < 5; ++b) {"
      "        putchar('.');"
      "     }"
      "     putchar('\\n');"
      "  }"
      "}",
   .output =
      ".....\n"
      ".....\n"
      ".....\n",
},
{
   .name = "modulo",
   .compiles = true,
   .source =
      "int printf(const char*, ...);"
      "int main(void) {"
      "  int a = 42;"
      "  int b = 22;"
      "  printf(\"%d\", a % b);"
      "}",
   .output = "20",
   .ret_val = 0
},
{
   .name = "sizeof(VLA)",
   .compiles = true,
   .source =
      "int printf(const char*, ...);"
      "unsigned get(void) { return 10; }"
      "int main(void) {"
      "  byte arr[get()];"
      "  printf(\"%u\", sizeof(arr));"
      "}",
   .output = "10",
},
{
   .name = "fibonacci",
   .compiles = true,
   .source = 
      "int fib(int n) {"
      "  return n <= 1 ? n : (fib(n - 1) + fib(n - 2));"
      "}"
      "int main(void) {"
      "  return fib(10);"
      "}",
   .ret_val = 55,
},
{
   .name = "pointer-arithmetic",
   .compiles = true,
   .source =
      "int printf(const char*, ...);"
      "int main(void) {"
      "  int* a = (int*)(sizeof(int) * 12);"
      "  int* b = (int*)(sizeof(int) * 10);"
      "  printf(\"%d\", a - b);"
      "}",
   .output = "2",
},
{
   .name = "enum",
   .compiles = true,
   .source =
      "enum values {"
      "  VAL_A,"
      "  VAL_B,"
      "};"
      "int main(void) {"
      "  return VAL_B;"
      "}",
   .ret_val = 1,
},
{
   .name = "extern-var",
   .compiles = true,
   .source =
      "extern int val;"
      "int val = 42;"
      "int main(void) { return val; }",
   .ret_val = 42,
},
{
   .name = "multiple definitions of f",
   .compiles = false,
   .source =
      "int f(void) { return 32; }"
      "int f(void) { return 42; }"
      "int main(void) { return f(); }",
},
{
   .name = "undefined type global variable",
   .compiles = false,
   .source =
      "undefined a;"
      "int main() {}",
},
{
   .name = "undefined type local variable",
   .compiles = false,
   .source =
      "int main() { undefined a; }",
},
{
   .name = "undefined type function",
   .compiles = false,
   .source =
      "undefined a();"
      "int main() {}",
},
{
   .name = "undefined type parameter",
   .compiles = false,
   .source =
      "int main(undefined a) {}"
},
{
   .name = "unnamed enum",
   .compiles = true,
   .source =
      "enum {"
      "  VAL_A,"
      "};"
      "enum {"
      "  VAL_B"
      "};"
      "int main(void) { return VAL_B; }",
},
{
   .name = "multiple definitions of enum",
   .compiles = false,
   .source =
      "enum A {};"
      "enum A {};"
      "int main(void) {}"
},
{
   .name = "multiple declarations of local variables",
   .compiles = false,
   .source =
      "int main(void) {"
      "  int a;"
      "  int a;"
      "}",
},
{
   .name = "declaration of variable+parameter with the same name",
   .compiles = false,
   .source =
      "int main(int argc) {"
      "  int argc;"
      "}"
},
{
   .name = "multiple parameters with the same name",
   .compiles = false,
   .source =
      "int main(int argc, int argc) {"
      "}"
},
{
   .name = "multi-variable declaration",
   .compiles = true,
   .source =
      "int main(void) {"
      "  int a, b;"
      "  a = 32;"
      "  b = 45;"
      "  return a + b;"
      "}",
   .ret_val = 77,
},
{
   .name = "declaration of int+array",
   .compiles = true,
   .source =
      "int main(void) {"
      "  int a[10], b;"
      "  b = arraylen(a);"
      "  return b;"
      "}",
   .ret_val = 10,
},
{
   .name = "multi-global-variable declaration",
   .compiles = true,
   .source =
      "int a = 13, b = 31;"
      "int main(void) {"
      "  return a + b;"
      "}",
   .ret_val = 44,
},
{
   .name = "global declaration of int+array",
   .compiles = true,
   .source =
      "int a[10], b;"
      "int main(void) {"
      "  return (sizeof(a) == (sizeof(int) * 10)) + (sizeof(b) == sizeof(int));"
      "}",
   .ret_val = 2,
},
{
   .name = "comma-expression",
   .compiles = true,
   .source =
      "int main(void) {"
      "  return 42, 3;"
      "}",
   .ret_val = 3,
},
{
   .name = "void-function",
   .compiles = true,
   .source =
      "int printf(const char*, ...);"
      "static void puti(int i) {"
      "  printf(\"%d\\n\", i);"
      "  return;"
      "}"
      "int main(void) {"
      "  puti(42);"
      "}",
   .output = "42\n",
},
{
   .name = "boolean and",
   .compiles = true,
   .source =
      "int main(void) {"
      "  int a = 3, b = 1;"
      "  return a && b;"
      "}",
   .ret_val = 1,
},
{
   .name = "boolean or",
   .compiles = true,
   .source =
      "int main(void) {"
      "  int a = 3, b = 1;"
      "  return a || b;"
      "}",
   .ret_val = 3,
},
{
   .name = "boolean not",
   .compiles = true,
   .source =
      "int main(void) {"
      "  int a = 1;"
      "  return !a;"
      "}",
   .ret_val = 0,
},
{
   .name = "typedef unnamed enum",
   .compiles = true,
   .source =
      "typedef enum {"
      "  VAL_A,"
      "  VAL_B,"
      "} e_t;"
      "int main(void) {"
      "  e_t e = VAL_B;"
      "  return e;"
      "}",
   .ret_val = 1,
},
{
   .name = "typedef enum",
   .compiles = true,
   .source =
      "enum E {"
      "  VAL_A,"
      "  VAL_B,"
      "};"
      "typedef enum E e_t;"
      "int main(void) {"
      "  e_t e = VAL_B;"
      "  return e;"
      "}",
   .ret_val = 1,
},
{
   .name = "sizeof(struct)",
   .compiles = true,
   .source =
      "struct A {"
      "  int a;"
      "  int b;"
      "};"
      "int main(void) {"
      "  struct A a;"
      "  return sizeof(a) / sizeof(int);"
      "}",
   .ret_val = 2,
},
{
   .name = "struct-member",
   .compiles = true,
   .source =
      "struct A {"
      "  int a;"
      "  int b;"
      "};"
      "int main(void) {"
      "  struct A a;"
      "  a.a = 42;"
      "  return a.a;"
      "}",
   .ret_val = 42,
},
{
   .name = "member-of-int",
   .compiles = false,
   .source =
      "int main(void) {"
      "  int a = 42;"
      "  return a.i;"
      "}"
},
