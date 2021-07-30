
all: tester
	@./tester

tester: tester.c cases.h
	gcc -o $@ $< -Wall -Wextra -std=c99 -D_POSIX_SOURCE=700 -O3

clean:
	rm -f tester bcc.log gcc.log

.PHONY: all clean
