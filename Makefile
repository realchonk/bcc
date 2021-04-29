CC=cc
CFLAGS=-c -g -std=c99 -Og -Iinclude -Wall -Wextra -D_XOPEN_SOURCE=700 -Wno-missing-braces

TARGET ?= i386

LD=$(CC)
LDFLAGS=
LIBS=-lm

DESTDIR ?= /usr/local
BINDIR ?= bin
MANDIR ?= share/man/man1

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
	make -C test clean

todo:
	@grep -n TODO $(sources) $(includes) || true

install:
	install -Dm755 bcc $(DESTDIR)/$(BINDIR)/bcc
	install -Dm644 bcc.1 $(DESTDIR)/$(MANDIR)/bcc.1

test: bcc
	make -C test run


.PHONY: all clean todo install test
