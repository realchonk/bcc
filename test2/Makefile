#  Copyright (C) 2021 Benjamin Stürz
#  
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#  
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <https://www.gnu.org/licenses/>.


BCC_ARCH ?= $(shell ../bcc -dumparch)

BCCFLAGS+=-O2

ifeq ($(BCC_ARCH),x86_64)
BITS=64
ARCH=x86
else ifeq ($(BCC_ARCH),i386)
BITS=32
ARCH=x86
else ifeq ($(BCC_ARCH),riscv32)
BITS=32
ARCH=riscv
QLP=/opt/riscv/riscv32-buildroot-linux-gnu/sysroot/
BCCFLAGS+=-mabi=ilp32d
else ifeq ($(BCC_ARCH),riscv64)
BITS=64
ARCH=riscv
QLP=/opt/riscv64/riscv64-buildroot-linux-musl/sysroot/
BCCFLAGS+=-mabi=lp64
endif

ifeq ($(BCC_ARCH),$(shell uname -m))
GCC=gcc
else
GCC=$(BCC_ARCH)-linux-gcc
endif

all: test

test.ir: test.c ../bcc
	../bcc -i -o $@ $< $(BCCFLAGS)

ifeq ($(ARCH),x86)
test.asm: test.c ../bcc
	../bcc -S -o $@ $< $(BCCFLAGS)

test.o: test.asm
	../bcc -c -o $@ $<

test: test.o
	$(GCC) -m$(BITS) -o $@ $<

else ifeq ($(ARCH),riscv)

test: test.o
	$(GCC) -o $@ $<

test.o: test.s
	../bcc -c -o $@ $< $(BCCFLAGS)

test.s: test.c ../bcc
	../bcc -S -o $@ $< $(BCCFLAGS)
endif

clean:
	rm -f test.s test.asm test.o test test.ir

run: test
	@QEMU_LD_PREFIX=$(QLP) ./test;

.PHONY: all clean
