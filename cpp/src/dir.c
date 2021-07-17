#include <string.h>
#include "buf.h"
#include "dir.h"


static struct directive dirs[] = {
   {
      .name = "define",
      .handler = dir_define,
   },
};

struct directive* get_dir(const char* name, size_t len) {
   for (size_t i = 0; i < arraylen(dirs); ++i) {
      if (!strncmp(name, dirs[i].name, len))
         return &dirs[i];
   }
   return NULL;
}
