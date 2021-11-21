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

#include <string.h>
#include <stdio.h>
#include "cmdline.h"
#include "error.h"
#include "cpu.h"
#include "buf.h"

const struct arm_cpu arm_cpus[] = {
#include "cpus.h"
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

