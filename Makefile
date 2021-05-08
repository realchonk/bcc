VER="0.5"

ifeq ($(TARGET),)
ARCH=$(shell ./util/getarch.sh)
TARGET=$(ARCH)
else
ARCH=$(shell ./util/getarch.sh $(TARGET))
endif

CC=cc -c -g -std=c99 -Og
CFLAGS += -Iinclude -Wall -Wextra -D_XOPEN_SOURCE=700 -Wno-missing-braces -Wno-array-bounds
CFLAGS += -DBCC_ARCH=\"$(ARCH)\" -DBCC_$(ARCH)=1 -DBCC_VER=\"$(VER)\"

LD=cc
LDFLAGS=
LIBS=-lm

PREFIX ?= /usr/local
BINDIR ?= /bin
MANDIR ?= /share/man/man1

includes=$(wildcard include/*.h)
sources=$(wildcard src/*.c) $(wildcard src/$(ARCH)/*.c)
objects=$(patsubst src/%.c,obj/%.o,$(wildcard src/*.c)) \
		  $(patsubst src/$(ARCH)/%.c,obj/$(ARCH)/%.o,$(wildcard src/$(ARCH)/*.c))
target_includes=$(wildcard src/$(ARCH)/*.h)

all: bcc

bcc: check_arch check_deps obj/$(ARCH) $(objects)
	$(LD) -o $@ $(objects) $(LDFLAGS) $(LIBS)

obj:
	mkdir -p obj

obj/$(ARCH): obj
	mkdir -p obj/$(ARCH)

obj/$(ARCH)/%.o: src/$(ARCH)/%.c $(includes) $(target_includes)
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

check: test
test:
	make -C test

check_arch:
	@!([ -z "$(ARCH)" ] && echo "Unsupported target architecture '$(TARGET)'")

check_deps:
	@if [ -f "./src/$(ARCH)/check_deps.sh" ]; then ./src/$(ARCH)/check_deps.sh >/dev/null; fi

.PHONY: all clean todo install test check check_arch check_deps
