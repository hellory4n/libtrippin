CC = clang
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Wshadow -fno-strict-aliasing -g -I. -O0

ifeq ($(asan),true)
	CFLAGS += -fsanitize=address
endif

# just copy for every example lmao
refcount: examples/refcount.c
	$(CC) $(CFLAGS) -o examples/refcount examples/refcount.c libtrippin.c

vectors: examples/vectors.c
	$(CC) $(CFLAGS) -o examples/vectors examples/vectors.c libtrippin.c

log: examples/log.c
	$(CC) $(CFLAGS) -o examples/log examples/log.c libtrippin.c

slice: examples/slice.c
	$(CC) $(CFLAGS) -o examples/slice examples/slice.c libtrippin.c

clean:
	rm examples/refcount examples/vectors examples/log examples/slice