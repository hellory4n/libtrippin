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

#ifndef TRIPPIN_H
#define TRIPPIN_H
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Idk why I added this
#define TRIPPIN_VERSION "v0.1.0"

// I love exploiting the compiler
#define TRIPPIN_DESTRUCTOR(func) __attribute__((cleanup(func)))

// Default destructor thingy
#define TRIPPIN_REF TRIPPIN_DESTRUCTOR(trippin_release)

// Makes it look more like a spicy modern language
#ifndef TRIPPIN_NO_SHORTHAND
#define t_ref TRIPPIN_REF
#define t_new(type) trippin_new(sizeof(type))
#define t_nil NULL
#endif

// Reference count deez.
typedef struct {
	size_t count;
} TrippinRefHeader;

// Literally just malloc with a check
void* trippin_new(size_t size);

// Increases the reference count
void trippin_retain(void* ptr);

// Decreases the reference count, and frees the data.
void trippin_release(void* ptr);

#ifdef __cplusplus
}
#endif

#endif
