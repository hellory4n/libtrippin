CC = clang
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -Wshadow -g -I. -O0

# just copy for every example lmao
refcount: examples/refcount.c
	$(CC) $(CFLAGS) -o examples/refcount examples/refcount.c libtrippin.c

clean:
	rm examples/refcount