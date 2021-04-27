CC=cc
CFLAGS=-c -g -std=c99 -Og -Iinclude -Wall -Wextra -D_XOPEN_SOURCE=700 -Wno-missing-braces

TARGET ?= i386

LD=$(CC)
LDFLAGS=
LIBS=-lm

includes=$(wildcard include/*.h)
objects=$(patsubst src/%.c,obj/%.o,$(wildcard src/*.c)) \
		  $(patsubst src/$(TARGET)/%.c,obj/$(TARGET)/%.o,$(wildcard src/$(TARGET)/*.c))

all: bcc

bcc: obj/$(TARGET) $(objects)
	$(LD) -o $@ $(objects) $(LDFLAGS) $(LIBS)

obj:
	mkdir -p obj

obj/$(TARGET): obj
	mkdir -p obj/$(TARGET)

obj/%.o: src/%.c $(includes)
	$(CC) -o $@ $< $(CFLAGS)

clean:
	rm -rf obj

todo:
	@grep -n TODO $(sources) $(includes) || true

.PHONY: all clean
