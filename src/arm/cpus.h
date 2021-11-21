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

// Feature description for supported ARM CPUs.
// This file must not be moved or renamed.
// Otherwise change `src/arm/cpu.c` and `src/arm/detect_sys.sh`

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
