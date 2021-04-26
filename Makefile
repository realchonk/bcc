CC=cc
CFLAGS=-c -g -std=c99 -Og -Iinclude -Wall -Wextra -D_XOPEN_SOURCE=700 -Wno-missing-braces

LD=$(CC)
LDFLAGS=
LIBS=-lm

sources=$(wildcard src/*.c)
includes=$(wildcard include/*.h)
objects=$(patsubst src/%.c,obj/%.o,$(sources))

all: bcc

bcc: obj $(objects)
	$(LD) -o $@ $(objects) $(LDFLAGS) $(LIBS)

obj:
	mkdir -p obj

obj/%.o: src/%.c $(includes)
	$(CC) -o $@ $< $(CFLAGS)

clean:
	rm -rf obj

todo:
	@grep -n TODO $(sources) $(includes) || true

.PHONY: all clean
