
all: tester
	@./tester

tester: tester.c cases.h
	gcc -o $@ $< -Wall -Wextra -O3


clean:
	rm -f tester

.PHONY: all run clean
