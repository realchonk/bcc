
all: run_tests
	@./run_tests

run_tests: tester.c cases.h
	gcc -o $@ $< -Wall -Wextra -O3 -std=c99 -D_POSIX_SOURCE=700


clean:
	rm -f run_tests

.PHONY: all run clean
