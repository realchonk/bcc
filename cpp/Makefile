include ../util/config.mk

sources=$(wildcard src/*.c)
objects=$(patsubst src/%.c,obj/%.o,$(sources))

all: bcc-cpp

bcc-cpp: obj $(objects)
	$(CC) -o $@ $(objects) $(CFLAGS)

obj:
	mkdir -p obj

obj/%.o: src/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -rf obj

.PHONY: all clean
