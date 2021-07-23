include ../util/config.mk

sources=$(wildcard src/*.c)
objects=$(patsubst src/%.c,obj/%.o,$(sources))

TOP ?= ..

all: bcpp

bcpp: include/help_options.h $(objects)
	$(CC) -o $@ $(objects) $(CFLAGS)

obj/%.o: src/%.c include
	@mkdir -p obj
	$(CC) -c -o $@ $< $(CFLAGS)

include/help_options.h: bcpp.1
	$(TOP)/util/read_doc.sh <$< >$@

clean:
	rm -f bcpp include/help_options.h
	rm -rf obj

.PHONY: all clean
