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
#include <__stdarg_va_list.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// Idk why I added this
#define TRIPPIN_VERSION "v0.1.0"

// Reference count deez.
typedef struct {
	size_t count;
} TrippinRefHeader;

// It initializes libtrippin.
void trippin_init(const char* log_file);

// It frees libtrippin.
void trippin_free(void);

// I love exploiting the compiler
#define TRIPPIN_DESTRUCTOR(func) __attribute__((cleanup(func)))

// arenas

// Kilobytes to bytes
#define TRIPPIN_KB(b) ((b) * 1024)
// Megabytes to bytes
#define TRIPPIN_MB(b) (TRIPPIN_KB((b)) * 1024)
// Gigabytes to bytes
#define TRIPPIN_GB(b) (TRIPPIN_MB((b)) * 1024)

typedef struct {
	size_t size;
	size_t alloc_pos;
	void* buffer;
} TrippinArena;

// Makes a new arena :)
TrippinArena trippin_arena_new(size_t size);

// Frees the arena and everything inside it.
void trippin_arena_free(TrippinArena arena);

// Allocates space in the arena.
void* trippin_arena_alloc(TrippinArena arena, size_t size);

// vectors

typedef struct {
	double x;
	double y;
} TrippinVec2f;

typedef struct {
	int64_t x;
	int64_t y;
} TrippinVec2i;

typedef struct {
	double x;
	double y;
	double z;
} TrippinVec3f;

typedef struct {
	int64_t x;
	int64_t y;
	int64_t z;
} TrippinVec3i;

#define TRIPPIN_ADDV2(a, b)  {a.x + b.x, a.y + b.y}
#define TRIPPIN_SUBV2(a, b)  {a.x - b.x, a.y - b.y}
#define TRIPPIN_MULV2(a, b)  {a.x * b.x, a.y * b.y}
#define TRIPPIN_SMULV2(a, b) {a.x * b,   a.y * b}
#define TRIPPIN_DIVV2(a, b)  {a.x / b.x, a.y / b.y}
#define TRIPPIN_SDIVV2(a, b) {a.x / b,   a.y / b}

#define TRIPPIN_EQV2(a, b)  (a.x == b.x && a.y == b.y)
#define TRIPPIN_NEQV2(a, b) (a.x != b.x && a.y != b.y)
#define TRIPPIN_LTV2(a, b)  (a.x < b.x  && a.y < b.y)
#define TRIPPIN_LTEV2(a, b) (a.x <= b.x && a.y <= b.y)
// shout out to lua
#define TRIPPIN_GTV2(a, b)  TRIPPIN_LTV2(b, a)
#define TRIPPIN_GTEV2(a, b) TRIPPIN_LTEV2(b, a)

#define TRIPPIN_ADDV3(a, b)  {a.x + b.x, a.y + b.y, a.z + b.z}
#define TRIPPIN_SUBV3(a, b)  {a.x - b.x, a.y - b.y, a.z - b.z}
#define TRIPPIN_MULV3(a, b)  {a.x * b.x, a.y * b.y, a.z * b.z}
#define TRIPPIN_SMULV3(a, b) {a.x * b,   a.y * b,   a.z * b}
#define TRIPPIN_DIVV3(a, b)  {a.x / b.x, a.y / b.y, a.z / b.z}
#define TRIPPIN_SDIVV3(a, b) {a.x / b,   a.y / b,   a.z / b}

#define TRIPPIN_EQV3(a, b)  (a.x == b.x && a.y == b.y && a.z == b.z)
#define TRIPPIN_NEQV3(a, b) (a.x != b.x && a.y != b.y && a.z != b.z)
#define TRIPPIN_LTV3(a, b)  (a.x < b.x  && a.y < b.y  && a.z <  b.z)
#define TRIPPIN_LTEV3(a, b) (a.x <= b.x && a.y <= b.y && a.z <= b.z)
// shout out to lua
#define TRIPPIN_GTV3(a, b)  TRIPPIN_LTV3(b, a)
#define TRIPPIN_GTEV3(a, b) TRIPPIN_LTEV3(b, a)

// lmao
#ifndef TRIPPIN_NO_SHORTHAND
#define tv2_add(a, b)  TRIPPIN_ADDV2(a, b)
#define tv2_sub(a, b)  TRIPPIN_SUBV2(a, b)
#define tv2_mul(a, b)  TRIPPIN_MULV2(a, b)
#define tv2_smul(a, b) TRIPPIN_SMULV2(a, b)
#define tv2_div(a, b)  TRIPPIN_DIVV2(a, b)
#define tv2_sdiv(a, b) TRIPPIN_SDIVV2(a, b)
#define tv2_eq(a, b)   TRIPPIN_EQV2(a, b)
#define tv2_neq(a, b)  TRIPPIN_NEQV2(a, b)
#define tv2_lt(a, b)   TRIPPIN_LTV2(a, b)
#define tv2_lte(a, b)  TRIPPIN_LTEV2(a, b)
#define tv2_gt(a, b)   TRIPPIN_GTV2(a, b)
#define tv2_gte(a, b)  TRIPPIN_GTEV2(a, b)

#define tv3_add(a, b)  TRIPPIN_ADDV3(a, b)
#define tv3_sub(a, b)  TRIPPIN_SUBV3(a, b)
#define tv3_mul(a, b)  TRIPPIN_MULV3(a, b)
#define tv3_smul(a, b) TRIPPIN_SMULV3(a, b)
#define tv3_div(a, b)  TRIPPIN_DIVV3(a, b)
#define tv3_sdiv(a, b) TRIPPIN_SDIVV3(a, b)
#define tv3_eq(a, b)   TRIPPIN_EQV3(a, b)
#define tv3_neq(a, b)  TRIPPIN_NEQV3(a, b)
#define tv3_lt(a, b)   TRIPPIN_LTV3(a, b)
#define tv3_lte(a, b)  TRIPPIN_LTEV3(a, b)
#define tv3_gt(a, b)   TRIPPIN_GTV3(a, b)
#define tv3_gte(a, b)  TRIPPIN_GTEV3(a, b)
#endif

// logging

#define TRIPPIN_CONSOLE_COLOR_RESET    "\033[0m"
#define TRIPPIN_CONSOLE_COLOR_LIB_INFO "\033[0;90m"
#define TRIPPIN_CONSOLE_COLOR_WARN     "\033[0;93m"
#define TRIPPIN_CONSOLE_COLOR_ERROR    "\033[0;91m"

typedef enum {
	// literally just for use with raylib
	TRIPPIN_LOG_LIB_INFO,
	TRIPPIN_LOG_INFO,
	TRIPPIN_LOG_WARNING,
	TRIPPIN_LOG_ERROR,
} TrippinLogLevel;

// Log without formatting
void trippin_print(TrippinLogLevel level, const char* text);

// Log.
void trippin_log(TrippinLogLevel level, const char* fmt, ...);

// Formatted assert?!!!??!?!??!?1
void trippin_assert(bool x, const char* msg, ...);

// uh oh
void trippin_panic(const char* msg, ...);

// slices

typedef struct {
	size_t length;
	size_t elem_size;
	void* buffer;
} TrippinSlice;

// Creates a new slice in an arena. The element size is supposed to be used with sizeof,
// e.g. sizeof(int) for a slice of ints.
TrippinSlice trippin_slice_new(TrippinArena arena, size_t length, size_t elem_size);

// Gets the element at the specified index. Note this returns a pointer to the element so this is also
// how you change elements.
void* trippin_slice_at(TrippinSlice slice, size_t idx);

// We love strings
typedef struct {
	// includes one extra character for the null terminator
	size_t length;
	char* buffer;
} TrippinStr;

// Makes a string duh. Adds 1 extra character for the null terminator so it's easier to pass it around
TrippinStr trippin_str_new(TrippinArena arena, const char* lit);

// Copies a string into a new one :)
TrippinStr trippin_str_copy(TrippinArena arena, TrippinStr str);

// Returns a new string with string B at the end of string A
TrippinStr trippin_str_concat(TrippinArena arena, TrippinStr a, TrippinStr b);

// If true, the 2 strings have the same contents.
bool trippin_str_equal(TrippinStr a, TrippinStr b);

// Substring lmao. This copies the string
TrippinStr trippin_str_substr(TrippinArena arena, TrippinStr str, size_t start, size_t len);

// Gets the character at the specified index. NOTE: This WILL break with unicode.
char trippin_str_at(TrippinStr str, size_t idx);

// Creates a new string from the trippin string. If it's smaller than 512, allocates it on the stack.
// Otherwise, it's allocated on the heap.
char* trippin_str_to_cstr(TrippinStr str);

#ifndef TRIPPIN_NO_SHORTHAND
#define tfree(func) TRIPPIN_DESTRUCTOR(func)
#define tpass(var) trippin_reference(var)
#define tstr(arena, str) trippin_str_new(arena, str)
#endif

#ifdef __cplusplus
}
#endif

#endif
