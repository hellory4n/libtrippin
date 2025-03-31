#include <stdio.h>
#include "libtrippin.h"

typedef struct {
	// required for the reference counter to work
	TrippinRef rc;
	// your own struct stuff goes after it
	int state_secrets;
} MyHandsomeStruct;

int main(void) {
	ref MyHandsomeStruct* mate = new(MyHandsomeStruct);
	return 0;
}
