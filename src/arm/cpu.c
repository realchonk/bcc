#include <string.h>
#include <stdio.h>
#include "cmdline.h"
#include "error.h"
#include "cpu.h"
#include "buf.h"

const struct arm_cpu arm_cpus[] = {
   {
      .name = "armv4",
      .features = (const char*[]){
         NULL
      },
   },
   {
      .name = "armv5",
      .features = (const char*[]){
         "bx",
         "blx",
         NULL
      },
   },
   {
      .name = "armv7",
      .features = (const char*[]){
         "bx",
         "blx",
         "movw/t",
         NULL
      },
   },
};
const size_t num_arm_cpus = arraylen(arm_cpus);

static const struct arm_cpu* get_cpu(void) {
   const char* name = get_mach_opt("cpu")->sVal;
   for (size_t i = 0; i < num_arm_cpus; ++i) {
      if (!strcmp(name, arm_cpus[i].name))
         return &arm_cpus[i];
   }
   panic("Invalid CPU: '%s'", name);
}

bool cpu_has_feature(const char* feature) {
   const struct arm_cpu* cpu = get_cpu();
   for (size_t i = 0; cpu->features[i]; ++i) {
      if (!strcmp(feature, cpu->features[i]))
         return true;
   }
   return false;
}

bool emit_prepare(void) {
   const char* name = get_mach_opt("cpu")->sVal;
   for (size_t i = 0; i < num_arm_cpus; ++i) {
      if (!strcmp(name, arm_cpus[i].name))
         return true;
   }
   fprintf(stderr, "bcc: invalid CPU '%s'\n", name);
   fprintf(stderr, "Available CPUS:");
   for (size_t i = 0; i < num_arm_cpus; ++i) {
      fprintf(stderr, " %s", arm_cpus[i].name);
   }
   fputc('\n', stderr);
   return false;
}

