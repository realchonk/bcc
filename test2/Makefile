

ARCH ?= $(shell ../bcc -dumpmachine)

BCCFLAGS+=-O2

ifeq ($(ARCH),x86_64)
BITS=64
ARCH=x86
else ifeq ($(ARCH),i386)
BITS=32
ARCH=x86
else ifeq ($(ARCH),riscv32)
BITS=32
ARCH=riscv
QLP=/opt/riscv/riscv32-buildroot-linux-gnu/sysroot/
BCCFLAGS+=-mabi=ilp32d
else ifeq ($(ARCH),riscv64)
BITS=64
ARCH=riscv
QLP=/opt/riscv64/riscv64-buildroot-linux-musl/sysroot/
BCCFLAGS+=-mabi=lp64
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
	riscv$(BITS)-linux-gcc -o $@ $<

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

