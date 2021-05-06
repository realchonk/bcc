VER="0.4"
TARGET ?= $(shell uname -m)

CC=cc
CFLAGS += -c -g -std=c99 -Og -Iinclude -Wall -Wextra -D_XOPEN_SOURCE=700 -Wno-missing-braces
CFLAGS += -DBCC_ARCH=\"$(TARGET)\" -DBCC_$(TARGET)=1 -DBCC_VER=\"$(VER)\"

LD=$(CC)
LDFLAGS=
LIBS=-lm

PREFIX ?= /usr/local
BINDIR ?= /bin
MANDIR ?= /share/man/man1

includes=$(wildcard include/*.h)
sources=$(wildcard src/*.c) $(wildcard src/$(TARGET)/*.c)
objects=$(patsubst src/%.c,obj/%.o,$(wildcard src/*.c)) \
		  $(patsubst src/$(TARGET)/%.c,obj/$(TARGET)/%.o,$(wildcard src/$(TARGET)/*.c))
target_includes=$(wildcard src/$(TARGET)/*.h)

all: bcc

bcc: obj/$(TARGET) $(objects)
	$(LD) -o $@ $(objects) $(LDFLAGS) $(LIBS)

obj:
	mkdir -p obj

obj/$(TARGET): obj
	mkdir -p obj/$(TARGET)

obj/$(TARGET)/%.o: src/$(TARGET)/%.c $(includes) $(target_includes)
	$(CC) -o $@ $< $(CFLAGS)

obj/%.o: src/%.c $(includes)
	$(CC) -o $@ $< $(CFLAGS)

clean:
	rm -rf obj
	make -C test clean

todo:
	@grep -n TODO $(sources) $(includes) || true

install:
	install -Dm755 bcc $(PREFIX)/$(BINDIR)/bcc
	install -Dm644 bcc.1 $(PREFIX)/$(MANDIR)/bcc.1

test:
	make -C test


.PHONY: all clean todo install test
