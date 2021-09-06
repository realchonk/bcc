//  Copyright (C) 2021 Benjamin Stürz
//  
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//  
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

#define _GNU_SOURCE
#include <stdnoreturn.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#define arraylen(a) (sizeof(a) / sizeof(*(a)))

#define COLOR_BOLD_GREEN   "\033[1;32m"
#define COLOR_BOLD_RED     "\033[1;31m"
#define COLOR_GREEN        "\033[32m"
#define COLOR_RED          "\033[31m"

#if defined(__x86_64__) || defined(__i386__)
#define BCC_ADD "-mstack-check"
#else
#define BCC_ADD
#endif

#define PATH_BCC "../bcc"
#define BCC PATH_BCC " -Cv -e ../cpp/bcpp -I ../bcc-include -O2 -w -L ../libbcc -nobccobjs " BCC_ADD " -o "
#define TEST_SOURCE "/tmp/test.c"
#define TEST_BINARY "/tmp/test"

// 0 = silent, 1 = default, 2 = verbose
static int verbosity = 1;

struct test_case {
   const char* name;
   bool compiles;
   const char* source;
   const char* output;
   int ret_val;
};

static struct test_case cases[] = {
#include "cases.h"
};

static noreturn void panic(const char* msg, ...) {
   va_list ap;
   va_start(ap, msg);

   fputs("bcc: ", stderr);
   vfprintf(stderr, msg, ap);
   if (errno) {
      fprintf(stderr, ": %s\n", strerror(errno));
   } else fputc('\n', stderr);

   va_end(ap);
   exit(2);
}

static void write_test(const char* s) {
   FILE* file = fopen(TEST_SOURCE, "w");
   if (!file) panic("failed to open '%s'", TEST_SOURCE);
   fwrite(s, 1, strlen(s), file);
   fclose(file);
}

static int compile_test(void) {
   const char* bcc_str;
   if (verbosity < 2) {
      bcc_str = BCC TEST_BINARY " " TEST_SOURCE " 2>>bcc.log";
   } else {
      // TODO: fix this, the exit value of bcc is discarded
      bcc_str = BCC TEST_BINARY " " TEST_SOURCE " 2>&1 | tee -a bcc.log";
   }
   int ec = system(bcc_str);
   if (ec < 0 || ec == 127) panic("failed to invoke bcc");
   else if (ec != 0) return ec;

   return ec;
}

static bool run_test(const struct test_case* test) {
   const char* cause;
   if (verbosity >= 2)
      printf("Running test case '%s'\n", test->name);
   write_test(test->source);
   const int compiler_ec = compile_test();
   if (compiler_ec == 139) {
      cause = "compiler crashed";
      goto failed;
   }
   if (!compiler_ec != test->compiles) {
      if (!compiler_ec) cause = "invalid compilation success";
      else cause = "failed to compile";
      goto failed;
   } else if (!test->compiles) goto success;

   bool r = true;
   const size_t len = test->output ? strlen(test->output) : 0;
   if (len) {
      int pipes[2];
      if (pipe(pipes) != 0) panic("failed to pipe()");
      pid_t pid = fork();
      if (pid < 0) panic("failed to fork()");
      if (pid == 0) {
         close(pipes[0]);                       // close read end
         close(STDOUT_FILENO);                  // close stdout
         if (dup(pipes[1]) != STDOUT_FILENO) {  // redirect stdout to pipes[1]
            fputs("failed to redirect", stderr);
            _exit(1);
         }  
         execl(TEST_BINARY, TEST_BINARY, NULL);
         if (verbosity > 0)
            fprintf(stderr, "failed to execl %s: %s\n", TEST_BINARY, strerror(errno));
         _exit(1);
      } else {
         int wstatus;
         close(pipes[1]);

         char* buffer = malloc(len + 1);
         int tmp;
         if ((size_t)read(pipes[0], buffer, len) != len
            || memcmp(buffer, test->output, len)
            || read(pipes[0], &tmp, 1) == 1) {
            r = false;
            cause = "output did not match";
         }
         free(buffer);

         waitpid(pid, &wstatus, 0);
         if (!WIFEXITED(wstatus)) {
            r = false;
            cause = "process did not exit properly";
         } else if (WEXITSTATUS(wstatus) != test->ret_val) {
            r = false;
            cause = "return value did not match";
         }
      }
   } else {
      pid_t pid = fork();
      if (pid == 0) {
         execl(TEST_BINARY, TEST_BINARY, NULL);
         fprintf(stderr, "tester: failed to execl: %s\n", strerror(errno));
         _exit(1);
      } else {
         int wstatus;
         waitpid(pid, &wstatus, 0);
         if (WIFEXITED(wstatus)) {
            r = WEXITSTATUS(wstatus) == test->ret_val;
            if (!r) cause = "return value did not match";
         } else {
            r = false;
            cause = "failed to invoke test";
         }
      }
   }

   if (!r) goto failed;
success:
   if (verbosity > 0)
      printf(COLOR_BOLD_GREEN "TEST '%s' %s\033[0m\n", test->name, test->compiles ? "PASS" : "XFAIL");
   return true;
failed:
   if (verbosity > 0) {
      printf(COLOR_BOLD_RED "TEST '%s' FAIL\033[0m\n", test->name);
      printf(COLOR_RED "%s.\033[0m\n", cause);
   }
   return false;
}

static struct test_case* get_case(const char* name) {
   for (size_t i = 0; i < arraylen(cases); ++i) {
      if (!strcmp(name, cases[i].name)) return &cases[i];
   }
   return NULL;
}

int main(int argc, char* argv[]) {
   int option;
   while ((option = getopt(argc, argv, ":vsh")) != -1) {
      switch (option) {
      case 'v':
         verbosity = 2;
         break;
      case 's':
         verbosity = 0;
         break;

      case 'h':
         puts("Usage: tester [-vsh] [test...]");
         puts("Options:");
         puts(" -h            Show this help message");
         puts(" -s            Silence the output");
         puts(" -v            Show more verbose output");
         return 0;
      case ':':
         fprintf(stderr, "tester: missing argument for -%c\n", optopt);
         return 1;
      case '?':
         fprintf(stderr, "tester: invalid option -%c\n", optopt);
         return 1;
      }
   }
   remove("bcc.log");
   remove("gcc.log");
   if (optind == argc) {
      const size_t num = arraylen(cases);
      size_t failed = 0;
      for (size_t i = 0; i < num; ++i) {
         if (!run_test(&cases[i])) ++failed;
      }

      if (verbosity > 0)
         printf("%s%zu out of %zu tests passed\033[0m\n", failed ? COLOR_RED : COLOR_GREEN, num - failed, num);
      return failed ? 2 : 0;
   } else {
      int ec = 0;
      for (; optind < argc; ++optind) {
         const char* name = argv[optind];
         struct test_case* t = get_case(name);
         if (!t) {
            printf(COLOR_RED "TEST '%s' NOT FOUND\033[0m", name);
            return 1;
         }
         if (!run_test(t))
            ec = 2;
      }
      return ec;
   }
}
