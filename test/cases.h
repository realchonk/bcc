{
   .name = "simple printf",
   .compiles = true,
   .source =
      "int printf(const char*, ...);\n"
      "int main(void) {\n"
      "  printf(\"Hello World\\n\");\n"
      "}",
   .output = "Hello World\n",
   .ret_val = 0,
},
{
   .name = "sizeof(array)",
   .compiles = true,
   .source =
      "int main(void) {\n"
      "  int arr[10];\n"
      "  return sizeof(arr);\n"
      "}",
   .output = "",
   .ret_val = 40,
},
{
   .name = "arraylen(VLA)",
   .compiles = true,
   .source =
      "int printf(const char*, ...);\n"
      "unsigned get(void) { return 10; }\n"
      "int main(void) {\n"
      "  int arr[get()];\n"
      "  printf(\"%u\", arraylen(arr));\n"
      "}",
   .output = "10",
   .ret_val = 0,
},
{
   .name = "typedef",
   .compiles = true,
   .source =
      "typedef unsigned int uint_t;\n"
      "int main(void) {\n"
      "  return sizeof(uint_t);\n"
      "}",
   .output = "",
   .ret_val = 4,
},
{
   .name = "for-array",
   .compiles = true,
   .source =
      "int printf(const char*, ...);\n"
      "int main(void) {\n"
      "  int arr[8];\n"
      "  for (unsigned i = 0; i < arraylen(arr); ++i) {\n"
      "     arr[i] = i;\n"
      "  }\n"
      "  for (unsigned i = 0; i < arraylen(arr); ++i) {\n"
      "     printf(\"arr[%u]=%u\\n\", i, arr[i]);\n"
      "  }\n"
      "  return 42;\n"
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
      "int printf(const char*, ...);\n"
      "int main(void) {\n"
      "  int a = 42;\n"
      "  int b = 99;\n"
      "  printf(\"%d\\n\", (a * b) + (b / a));\n"
      "}",
   .output = "4160",
   .ret_val = 0
},
{
   .name = "casting of integers",
   .compiles = true,
   .source =
      "int printf(const char*, ...);\n"
      "int main(void) {\n"
      "  int a = -1;\n"
      "  printf(\"%d\\n\", (short)a);\n"
      "}",
   .output = "-1",
   .ret_val = 0
},
{
   .name = "nested for-loop",
   .compiles = true,
   .source =
      "int putchar(int);\n"
      "int main(void) {\n"
      "  for (int a = 0; a < 3; ++a) {\n"
      "     for (int b = 0; b < 5; ++b) {\n"
      "        putchar('.');\n"
      "     }\n"
      "     putchar('\\n');\n"
      "  }\n"
      "}",
   .output =
      ".....\n"
      ".....\n"
      ".....\n",
   .ret_val = 0,
},
{
   .name = "modulo",
   .compiles = true,
   .source =
      "int printf(const char*, ...);\n"
      "int main(void) {\n"
      "  int a = 42;\n"
      "  int b = 22;\n"
      "  printf(\"%d\", a % b);\n"
      "}",
   .output = "20",
   .ret_val = 0
},
{
   .name = "sizeof(VLA)",
   .compiles = true,
   .source =
      "int printf(const char*, ...);\n"
      "unsigned get(void) { return 10; }\n"
      "int main(void) {\n"
      "  int arr[get()];\n"
      "  printf(\"%u\", sizeof(arr));\n"
      "}",
   .output = "40",
   .ret_val = 0,
},
{
   .name = "fibonacci",
   .compiles = true,
   .source = 
      "int fib(int n) {\n"
      "  return n <= 1 ? n : (fib(n - 1) + fib(n - 2));\n"
      "}\n"
      "int main(void) {\n"
      "  return fib(10);\n"
      "}",
   .output = "",
   .ret_val = 55,
},
{
   .name = "pointer-arithmetic",
   .compiles = true,
   .source =
      "int printf(const char*, ...);\n"
      "int main(void) {\n"
      "  int* a = (int*)(sizeof(int) * 12);\n"
      "  int* b = (int*)(sizeof(int) * 10);\n"
      "  printf(\"%d\\n\", a - b);\n"
      "}",
   .output = "2\n",
   .ret_val = 0,
},
{
   .name = "enum",
   .compiles = true,
   .source =
      "enum values {\n"
      "  VAL_A,\n"
      "  VAL_B,\n"
      "};"
      "int main(void) {\n"
      "  return VAL_B;\n"
      "}",
   .output = "",
   .ret_val = 1,
},
{
   .name = "extern-var",
   .compiles = true,
   .source =
      "extern int val;\n"
      "int val = 42;\n"
      "int main(void) { return val; }",
   .output = "",
   .ret_val = 42,
},
{
   .name = "multiple definitions of f",
   .compiles = false,
   .source =
      "int f(void) { return 32; }\n"
      "inf f(void) { return 42: }\n"
      "int main(void) { return f(); }\n",
}

