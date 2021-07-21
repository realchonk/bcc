include ../util/config.mk

sources=$(wildcard src/*.c)
objects=$(patsubst src/%.c,obj/%.o,$(sources))

all: bcpp

bcpp: $(objects)
	$(CC) -o $@ $(objects) $(CFLAGS)

obj/%.o: src/%.c include
	@mkdir -p obj
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f bcpp
	rm -rf obj

.PHONY: all clean
