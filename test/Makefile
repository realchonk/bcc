
all: test

run: test
	@(./$<; echo "exit code: $$?")

test: test.o
	gcc -m32 -o $@ $<

test.o: test.asm
	nasm -felf32 -o $@ $<

test.asm: test.c
	../bcc -o $@ -S $<

clean:
	rm -f test test.o test.asm

.PHONY: all run clean
