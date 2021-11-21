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
//

#ifndef FILE_ARM_CPU_H
#define FILE_ARM_CPU_H
#include <stdbool.h>

struct arm_cpu {
   const char* name;
   const char** features;
};

extern const struct arm_cpu arm_cpus[];
extern const size_t num_arm_cpus;

bool cpu_has_feature(const char*);

#endif /* FILE_ARM_CPU_H */
