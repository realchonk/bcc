CC=cc
CFLAGS=-c -g -std=c99 -Og -Iinclude -Wall -Wextra -D_XOPEN_SOURCE=700

LD=$(CC)
LDFLAGS=
LIBS=-lm

sources=$(wildcard src/*.c)
objects=$(patsubst src/%.c,obj/%.o,$(sources))

all: bcc

bcc: obj $(objects)
	$(LD) -o $@ $(objects) $(LDFLAGS) $(LIBS)

obj:
	mkdir -p obj

obj/%.o: src/%.c include
	$(CC) -o $@ $< $(CFLAGS)

clean:
	rm -rf obj

.PHONY: all clean
