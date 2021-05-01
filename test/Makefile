
all: test test.ir

run: test
	@(./$<; echo "\nexit code: $$?")

test: test.o
	gcc -o $@ $<

test.o: test.asm
	nasm -felf64 -o $@ $<

test.asm: test.c ../bcc
	../bcc -S -o $@ $< -O1

test.ir: test.c ../bcc
	../bcc -i -o $@ $<

clean:
	rm -f test test.o test.asm test.ir

.PHONY: all run clean
