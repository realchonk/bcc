
all: test test.ir

TARGET = $(shell ../bcc -V | tail -n1 | awk '{print $$NF}')

ifeq ($(TARGET),i386)
ASFLAGS = -felf32
CC = cc -m32
else ifeq ($(TARGET),x86_64)
ASFLAGS = -felf64
CC = cc
else

endif

olevel=2

run: test
	@(./$<; echo "\nexit code: $$?")

test: test.o
	$(CC) -o $@ $<

test.o: test.asm
	nasm $(ASFLAGS) -o $@ $<

test.asm: test.c ../bcc
	../bcc -S -o $@ $< -O $(olevel)

test.ir: test.c ../bcc
	../bcc -i -o $@ $< -O $(olevel)

clean:
	rm -f test test.o test.asm test.ir

.PHONY: all run clean
