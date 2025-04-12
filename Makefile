CC = clang
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -Wshadow -fno-strict-aliasing -I. -O0 -lm

ifeq ($(debug),true)
	CFLAGS += -fsanitize=address -DDEBUG -g
endif

all: vectors log slice_arenas math

# just copy for every example lmao
vectors: examples/vectors.c
	$(CC) $(CFLAGS) -o examples/vectors examples/vectors.c libtrippin.c

log: examples/log.c
	$(CC) $(CFLAGS) -o examples/log examples/log.c libtrippin.c

slice_arenas: examples/slice_arenas.c
	$(CC) $(CFLAGS) -o examples/slice_arenas examples/slice_arenas.c libtrippin.c

math: examples/math.c
	$(CC) $(CFLAGS) -o examples/math examples/math.c libtrippin.c

# man
clean:
	rm -f log.txt examples/vectors examples/log examples/slice_arenas examples/math examples/vectors.exe examples/log.exe examples/slice_arenas.exe examples/math.exe
