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
      "  printf(\"%u\\n\", arraylen(arr));\n"
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
 

