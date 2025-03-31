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
	// pointer fuckery for less annoyance
	void* val = calloc(1, sizeof(TrippinRef) + size);
	if (val == NULL) {
		printf("libtrippin panic: couldn't allocate memory\n");
		fflush(stdout);
		exit(1);
	}
	// you don't get something and then immediately throw it in the trash
	trippin_retain(val);

	// without the TrippinRef header
	return (void*)((char*)val + sizeof(TrippinRef));
}

void trippin_retain(void* ptr)
{
	// go back to the header
	// scary!
	TrippinRef* rc = (TrippinRef*)((char*)ptr - sizeof(TrippinRef));
	rc->count++;
}

void trippin_release(void* ptr)
{
	// scary!
	TrippinRef* rc = (TrippinRef*)((char*)ptr - sizeof(TrippinRef));
	printf("%li\n", rc->count);
	rc->count--;
	if (rc->count <= 1) {
		printf("or is it\n");
		free(ptr);
	}
	printf("it's dead.\n");
}
