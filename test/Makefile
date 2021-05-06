
all: run_tests
	@./run_tests

run_tests: tester.c cases.h
	gcc -o $@ $< -Wall -Wextra -O3


clean:
	rm -f run_tests

.PHONY: all run clean
