#include <stdint.h>
#include <stdio.h>
#include "libtrippin.h"

typedef struct {
	TrippinRefHeader rc;
	// your own struct stuff goes after it
	int64_t state_secrets;
} MyHandsomeStruct;

int main(void)
{
	t_ref MyHandsomeStruct* mate = t_new(MyHandsomeStruct);
	mate->state_secrets = 69420;
	return 0;
}
