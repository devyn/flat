CC = clang
CFLAGS = -g -O2
OBJECTS = obj/flat.o obj/flat/dictionary.o

all: bin/flat

bin/flat: $(OBJECTS)
	mkdir -p bin
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

obj/%.o: src/%.c
	mkdir -p "$(shell dirname "$@")"
	$(CC) -Isrc $(CFLAGS) $< -c -o $@

clean:
	rm -f $(OBJECTS) bin/flat

.PHONY: all clean

