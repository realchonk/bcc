

ARCH ?= $(shell ../bcc -dumpmachine)

BCCFLAGS ?= -O2

ifeq ($(ARCH),x86_64)
BITS=64
ARCH=x86
else ifeq ($(ARCH),i386)
BITS=32
ARCH=x86
else ifeq ($(ARCH),riscv32)
BITS=32
ARCH=riscv
ABI=ilp32
else ifeq ($(ARCH),riscv64)
BITS=64
ARCH=riscv
ABI=lp64
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
	gcc -m$(BITS) -o $@ $<

else ifeq ($(ARCH),riscv)

test: test.o
	riscv$(BITS)-linux-gcc -o $@ $< -mabi=$(ABI)

test.o: test.s
	../bcc -c -o $@ $<

test.s: test.c ../bcc
	../bcc -S -o $@ $< $(BCCFLAGS)
endif

clean:
	rm -f test.s test.asm test.o test test.ir
run: test
	./test;

.PHONY: all clean

