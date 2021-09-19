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

#define _XOPEN_SOURCE 700
#include <sys/wait.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include "../include/buf.h"

// 0 = silent, 1 = default, 2 = verbose
static int verbosity = 1;
static char* path_bcc = "../bcc";
static char* path_bcpp = "../cpp/bcpp";
static char* path_test = "./test";
static char* path_startfiles = "../libbcc";
static bool no_colors = false;
static char** extra_args = NULL;

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

static const char* colors[] = {
   /*                */ "\033[0m",
   /* GREEN          */ "\033[32m",
   /* BOLD GREEN     */ "\033[1;32m",
   /* RED            */ "\033[31m",
   /* BOLD RED       */ "\033[1;31m",
   /* YELLOW         */ "\033[33m",
   /* BOLD YELLOW    */ "\033[1;33m",
};

static void trim_leading_nl(char** buf) {
   const size_t len = buf_len(*buf);
   if (len < 2)
      return;
   if ((*buf)[len - 2] == '\n')
      buf_remove(*buf, len - 2, 1);
}

static void print(int v, int c, const char* fmt, ...) {
   va_list ap;
   if (v > verbosity)
      return;
   va_start(ap, fmt);
   if (!no_colors)
      printf("%s", colors[c]);
   vprintf(fmt, ap);
   if (!no_colors)
      printf("%s", colors[0]);
   va_end(ap);
}

static int do_wait(const pid_t pid, char** text, const char* what) {
   int wstatus;
   waitpid(pid, &wstatus, 0);
   if (WIFEXITED(wstatus)) {
      return WEXITSTATUS(wstatus);
   }
   buf_puts(*text, what);
   buf_push(*text, ' ');
   if (WIFSIGNALED(wstatus)) {
      char str[40];
      snprintf(str, sizeof(str), "killed by signal %d", WTERMSIG(wstatus));
      buf_puts(*text, str);
   } else if (WIFSTOPPED(wstatus)) {
      char str[40];
      snprintf(str, sizeof(str), "stopped by signal %d", WSTOPSIG(wstatus));
      buf_puts(*text, str);
   } else if (WIFCONTINUED(wstatus)) {
      buf_puts(*text, "continued");
   } else {
      buf_puts(*text, "failed by undetermined cause");
   }
   buf_push(*text, '\0');
   return 254;
}

static int run_compiler(const char* source, char** output) {
   int pipes_t2c[2];
   int pipes_c2t[2];
   int tmp;
   tmp = pipe(pipes_t2c);
   assert(tmp == 0);
   tmp = pipe(pipes_c2t);
   assert(tmp == 0);

   const pid_t pid = fork();
   assert(pid >= 0);
   if (pid == 0) {
      close(pipes_t2c[1]);
      close(pipes_c2t[0]);

      close(STDIN_FILENO);
      tmp = dup(pipes_t2c[0]);
      assert(tmp == STDIN_FILENO);

      close(STDOUT_FILENO);
      tmp = dup(pipes_c2t[1]);
      assert(tmp == STDOUT_FILENO);

      close(STDERR_FILENO);
      tmp = dup(pipes_c2t[1]);
      assert(tmp == STDERR_FILENO);

      char** args = NULL;
      buf_push(args, path_bcc);
      buf_push(args, "-otest");
      buf_push(args, "-e");
      buf_push(args, path_bcpp);
      buf_push(args, "-L../libbcc");
      buf_push(args, "-I../bcc-include");
      buf_push(args, "-O2");
      buf_push(args, "-nobccobjs");
      if (no_colors)
         buf_push(args, "-C");

      char* b = NULL;
      buf_puts(b, path_startfiles);
      buf_push(b, '/');
      buf_puts(b, "crtbegin.o");
      buf_push(b, '\0');

      buf_push(args, b);

      if (verbosity > 2)
         buf_push(args, "-v");
      buf_push(args, "-");

      b = NULL;
      buf_puts(b, path_startfiles);
      buf_push(b, '/');
      buf_puts(b, "crtend.o");
      buf_push(b, '\0');

      buf_push(args, b);

      for (size_t i = 0; i < buf_len(extra_args); ++i)
         buf_push(args, extra_args[i]);

      buf_push(args, NULL);

      execv(path_bcc, args);
      perror("");
      _exit(255);
   } else {
      close(pipes_t2c[0]);
      close(pipes_c2t[1]);

      FILE* fout = fdopen(pipes_t2c[1], "w");
      assert(fout != NULL);
      fputs(source, fout);
      fclose(fout);
      
      FILE* fin = fdopen(pipes_c2t[0], "r");
      char* text = NULL;
      while ((tmp = fgetc(fin)) != EOF)
         buf_push(text, tmp);
      if (text)
         buf_push(text, '\0');
      fclose(fin);
      if (output)
         *output = text;

      return do_wait(pid, output, "compiler");
   }
}

static int run_program(char** output) {
   int pipes[2];
   int tmp;
   tmp = pipe(pipes);
   assert(tmp == 0);

   const pid_t pid = fork();
   assert(pid >= 0);
   if (pid == 0) {
      close(pipes[0]);
      close(STDOUT_FILENO);
      tmp = dup(pipes[1]);
      assert(tmp == STDOUT_FILENO);

      close(STDERR_FILENO);
      tmp = dup(pipes[1]);
      assert(tmp == STDERR_FILENO);

      close(STDIN_FILENO);
      tmp = open("/dev/null", O_RDONLY);
      assert(tmp == STDIN_FILENO);

      execl(path_test, path_test, NULL);
      perror("");
      _exit(255);
   } else {
      close(pipes[1]);

      FILE* fin = fdopen(pipes[0], "r");
      assert(fin != NULL);
      char* text = NULL;
      while ((tmp = fgetc(fin)) != EOF)
         buf_push(text, tmp);
      buf_push(text, '\0');
      fclose(fin);
      if (output)
         *output = text;

      return do_wait(pid, output, "compiler");
   }
}

static bool run_test(const struct test_case* c) {
   char* output;
   int ec = run_compiler(c->source, &output);
   print(2, 5, "Running test '%s'\n", c->name);
   if (output && ec < 255) {
      print(2, 5, "Compiler Output:\n%s", output);
   }
   switch (ec) {
   case 0:
      if (!c->compiles) {
         print(1, 4, "TEST '%s': unexpected compilation success\n", c->name);
         return false;
      }
      break;
   case 1:
      if (c->compiles) {
         print(1, 4, "TEST '%s': failed to compile\n", c->name);
         return false;
      } else {
         print(1, 2, "TEST '%s': XFAIL\n", c->name);
         return true;
      }
      break;
   case 255:
      print(1, 4, "TEST '%s': failed to invoke compiler: %s", c->name, output);
      exit(1);
   case 254:
      trim_leading_nl(&output);
      print(1, 4, "TEST '%s': %s\n", c->name, output);
      return false;
   default:
      print(1, 4, "TEST '%s': compiler exited with code '%d'\n", c->name, ec);
      return false;
   }
   buf_free(output);

   print(2, 5, "Running executable\n");

   ec = run_program(&output);
   if (output) {
      print(2, 5, "Executable Output:\n");
      print(2, 0, "%s", output);
   }

   if (ec == 254) {
      trim_leading_nl(&output);
      print(1, 4, "TEST '%s': %s\n", c->name, output);
      return false;
   }
   if (ec != c->ret_val) {
      print(1, 4, "TEST '%s': invalid exit code '%d'\n", c->name, ec);
      return false;
   }
   if (c->output && (strcmp(output, c->output) != 0)) {
      print(1, 4, "TEST '%s': invalid output\n", c->name);
      return false;
   }
   print(1, 2, "TEST '%s': PASS\n", c->name);
   return true;
}

static const struct test_case* find_case(const char* name) {
   for (size_t i = 0; i < arraylen(cases); ++i) {
      const struct test_case* c = &cases[i];
      if (!strcmp(name, c->name))
         return c;
   }
   return NULL;
}

int main(int argc, char* argv[]) {
   bool keep_test = false;
   bool always0 = false;
   int option;

   while ((option = getopt(argc, argv, ":vshc:e:t:f:k0CX:")) != -1) {
      switch (option) {
      case 'v':
         verbosity = 2;
         break;
      case 's':
         verbosity = 0;
         break;
      case 'c':
         path_bcc = optarg;
         break;
      case 'e':
         path_bcpp = optarg;
         break;
      case 't':
         path_test = optarg;
         break;
      case 'f':
         path_startfiles = optarg;
         break;
      case 'k':
         keep_test = true;
         break;
      case '0':
         always0 = true;
         break;
      case 'C':
         no_colors = true;
         break;
      case 'X':
         buf_push(extra_args, optarg);
         break;

      case 'h':
         puts("Usage: tester [Options] [test...]");
         puts("Options:");
         puts(" -h                        Show this help message");
         puts(" -s                        Silence the output");
         puts(" -v                        Show more verbose output");
         puts(" -k                        Keep the test executable");
         puts(" -0                        Return always with exit code 0");
         puts(" -C                        Disable color output");
         puts(" -c path_bcc               Path to the compiler");
         puts(" -e path_bcpp              Path to the pre-processor");
         puts(" -t path_test              Path to the test executable");
         puts(" -f path_startfiles        Path to the startfiles directory");
         puts(" -X option                 Pass option to bcc");
         return 0;
      case ':':
         fprintf(stderr, "tester: missing argument for -%c\n", optopt);
         return 1;
      case '?':
         fprintf(stderr, "tester: invalid option -%c\n", optopt);
         return 1;
      }
   }
   int ec;
   if (optind == argc) {
      const size_t num = arraylen(cases);
      size_t passed = 0;
      for (size_t i = 0; i < num; ++i) {
         if (run_test(&cases[i]))
            ++passed;
      }
      if (verbosity > 0) {
         print(1, passed == num ? 1 : 3, "%zu out of %zu tests passes.\n", passed, num);
      }
      ec = passed == num ? 0 : 2;
   } else {
      for (; optind < argc; ++optind) {
         const char* name = argv[optind];
         const struct test_case* c = find_case(name);
         if (!c) {
            print(1, 4, "TEST '%s': NOT FOUND", name);
            return 1;
         }
         if (!run_test(c))
            ec = 2;
      }
   }
   if (!keep_test) {
      remove(path_test);
   }
   return always0 ? 0 : ec;
}

