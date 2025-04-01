/*
 * libtrippin
 *
 * Some utilities craps because the C standard library is tiny
 * More information at https://github.com/hellory4n/libtrippin
 *
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to <https://unlicense.org/>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libtrippin.h"

void* trippin_new(size_t size)
{
	void* val = calloc(1, size);
	if (val == NULL) {
		printf("libtrippin panic: couldn't allocate memory\n");
		fflush(stdout);
		exit(1);
	}

	// you don't get something and then immediately throw it in the trash
	((TrippinRefHeader*)val)->count = 1;

	// scary!
	return val;
}

void trippin_retain(void* ptr)
{
	// scary!
	TrippinRefHeader* rc = ((TrippinRefHeader*)(ptr));
	printf("%zu\n", rc->count);
	rc->count++;
	printf("%zu\n", rc->count);
}

void trippin_release(void* ptr)
{
	void** actualptr = ptr;
	// scary!
	// apparently attribute cleanup returns a pointer to the pointer, not the actual pointer
	TrippinRefHeader* rc = ((TrippinRefHeader*)(*actualptr));
	printf("%zu\n", rc->count);
	rc->count--;
	printf("%zu\n", rc->count);
	if (rc->count <= 0) {
		printf("or is it\n");
		free(ptr);
		printf("it's dead.\n");
	}
	printf("may dead\n");
}
