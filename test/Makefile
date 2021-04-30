
all: test

run: test
	@(./$<; echo "exit code: $$?")

test: test.o
	gcc -m32 -o $@ $<

test.o: test.c
	../bcc -c -o $@ $<

clean:
	rm -f test test.o test.asm

.PHONY: all run clean
