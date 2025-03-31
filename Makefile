# all my makefiles are stolen.
CC = clang
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -Wshadow -fsanitize=address -g -I.

LIB_NAME = libtrippin
LIB_SRC = libtrippin.c
LIB_OBJ = $(LIB_SRC:.c=.o)
LIBRARY = lib$(LIB_NAME).a

EXAMPLES = examples/refcount

all: $(EXAMPLES)

$(LIBRARY): $(LIB_OBJ)
	ar rcs $(LIBRARY) $(LIB_OBJ)

# just copy that for each example
refcount: example1.o $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ $< -L. -l$(LIB_NAME)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(LIBRARY) $(LIB_OBJ) $(EXAMPLES) *.o

.PHONY: all clean
