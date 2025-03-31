# all my makefiles are stolen.
CC = clang
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -Wshadow -fsanitize=address -g -I.

LIB_NAME = trippin
LIB_SRC = libtrippin.o
LIB_OBJ = libtrippin.c
LIBRARY = libtrippin.a

EXAMPLES = examples/refcount

all: $(EXAMPLES) $(LIB_OBJ)

$(LIBRARY): $(LIB_OBJ)
	ar rcs $(LIBRARY) $(LIB_OBJ)

# just copy that for each example
refcount: examples/refcount.o $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ $< -L. -l$(LIB_NAME)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(LIBRARY) $(LIB_OBJ) $(EXAMPLES) *.o

.PHONY: all clean
