
all: run_tests
	@./run_tests

run_tests: tester.c cases.h
	gcc -o $@ $< -Wall -Wextra -std=c99 -D_POSIX_SOURCE=700 -O3

clean:
	rm -f run_tests

.PHONY: all clean
