
all: test test.ir

run: test
	@(./$<; echo "exit code: $$?")

test: test.o
	gcc -m32 -o $@ $<

test.o: test.asm
	nasm -felf32 -o $@ $<

test.asm: test.c
	../bcc -S -o $@ $<

test.ir: test.c
	../bcc -i -o $@ $<

clean:
	rm -f test test.o test.asm test.ir

.PHONY: all run clean
