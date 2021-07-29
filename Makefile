include util/config.mk

OLD_TARGET := $(TARGET)

ifeq ($(TARGET),)
ARCH=$(shell ./util/getarch.sh)
TARGET=$(ARCH)
else
ARCH=$(shell ./util/getarch.sh $(TARGET))
endif

ifeq ($(DISABLE_FP),y)
CFLAGS += -DDISABLE_FP=1
endif

CFLAGS += -Wno-missing-braces -Wno-array-bounds
CFLAGS += -DBCC_ARCH=\"$(ARCH)\" -DBCC_$(ARCH)=1
LIBS += -lm

PREFIX ?= /usr/local
BINDIR ?= /bin
MANDIR ?= /share/man/man1

PROGRAM_PREFIX ?= $(shell cat .program_prefix 2>/dev/null)

includes=$(wildcard include/*.h)
sources=$(wildcard src/*.c) $(wildcard src/$(ARCH)/*.c)
objects=$(patsubst src/%.c,obj/%.o,$(wildcard src/*.c)) \
		  $(patsubst src/$(ARCH)/%.c,obj/$(ARCH)/%.o,$(wildcard src/$(ARCH)/*.c))
target_includes=$(wildcard src/$(ARCH)/*.h)

all: bcc bcpp

bcc: check_deps include/help_options.h $(objects)
	$(CC) -o $@ $(objects) $(CFLAGS) $(LIBS)
	@if [ -z "$(OLD_TARGET)" ]; then rm -f .program_prefix; else echo "$(OLD_TARGET)-" > .program_prefix; fi

bcpp:
	$(MAKE) TOP=$(PWD) -C cpp


include/help_options.h: bcc.1
	./util/read_doc.sh <$< >$@

obj/$(ARCH)/%.o: src/$(ARCH)/%.c $(includes) $(target_includes)
	@mkdir -p obj/$(ARCH)
	$(CC) -c -o $@ $< $(CFLAGS)

obj/%.o: src/%.c $(includes)
	@mkdir -p obj
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -rf obj
	rm -f bcc
	rm -f include/help_options.h .program_prefix
	rm -f src/riscv32/as.h
	make -C test clean
	make -C cpp clean
	make -C test2 clean

todo:
	@grep --color=yes -rn TODO src include cpp/src cpp/include || true

install:
	install -Dm755 bcc $(PREFIX)/$(BINDIR)/$(PROGRAM_PREFIX)bcc
	install -Dm755 cpp/bcpp $(PREFIX)/$(BINDIR)/bcpp
	install -Dm644 bcc.1 $(PREFIX)/$(MANDIR)/$(PROGRAM_PREFIX)bcc.1
	install -Dm644 cpp/bcpp.1 $(PREFIX)/$(MANDIR)/bcpp.1
	install -Dm644 util/bcc.bash $(PREFIX)/share/bash-completion/completions/$(PROGRAM_PREFIX)bcc
	install -Dm644 util/bcpp.bash $(PREFIX)/share/bash-completion/completions/bcpp
	sed -i "s/VERSION/$(VER)/g" $(PREFIX)/$(MANDIR)/$(PROGRAM_PREFIX)bcc.1
	sed -i "s/VERSION/$(VER)/g" $(PREFIX)/$(MANDIR)/bcpp.1

check: test
test:
	make -C test

check_arch:
	@!([ -z "$(ARCH)" ] && echo "Unsupported target architecture '$(TARGET)'")

check_deps: check_arch
	@if [ -f "./src/$(ARCH)/check_deps.sh" ]; then TOP=$(PWD) ARCH=$(ARCH) ./src/$(ARCH)/check_deps.sh >/dev/null; fi

.PHONY: all clean todo install test check check_arch check_deps
