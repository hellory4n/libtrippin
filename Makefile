CC = clang
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Wshadow -fno-strict-aliasing -I. -O0

ifeq ($(debug),true)
	CFLAGS += -fsanitize=address -DDEBUG -g
endif

# just copy for every example lmao
refcount: examples/refcount.c
	$(CC) $(CFLAGS) -o examples/refcount examples/refcount.c libtrippin.c

vectors: examples/vectors.c
	$(CC) $(CFLAGS) -o examples/vectors examples/vectors.c libtrippin.c

log: examples/log.c
	$(CC) $(CFLAGS) -o examples/log examples/log.c libtrippin.c

slice_arenas: examples/slice_arenas.c
	$(CC) $(CFLAGS) -o examples/slice_arenas examples/slice_arenas.c libtrippin.c

clean:
	rm examples/refcount examples/vectors examples/log examples/slice