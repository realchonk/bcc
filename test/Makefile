
all: test test.ir

run: test
	@(./$<; echo "\nexit code: $$?")

test: test.o
	gcc -m32 -o $@ $<

test.o: test.asm
	nasm -felf32 -o $@ $<

test.asm: test.c ../bcc
	../bcc -S -o $@ $< -O1

test.ir: test.c ../bcc
	../bcc -i -o $@ $<

clean:
	rm -f test test.o test.asm test.ir

.PHONY: all run clean
