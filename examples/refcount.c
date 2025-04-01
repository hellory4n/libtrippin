#include <stdint.h>
#include <stdio.h>
#include "libtrippin.h"

typedef struct {
	TrippinRefHeader rc;
	// your own struct stuff goes after it
	int64_t state_secrets;
} MyHandsomeStruct;

static void use_state_secrets(MyHandsomeStruct* ptr)
{
	// so the reference count goes down
	tref MyHandsomeStruct* _ = ptr;

	printf("state secrets: %li\n", ptr->state_secrets);
	printf("references: %zu\n", ptr->rc.count);
}

int main(void)
{
	// if you run `make asan=true refcount && ./examples/refcount`
	// you'll see there's no leaks
	tref MyHandsomeStruct* mate = tnew(MyHandsomeStruct);
	mate->state_secrets = 69420;
	// passing references and increasing the reference count
	// that's the point of a reference counter
	use_state_secrets(tpass(mate));
	return 0;
}
