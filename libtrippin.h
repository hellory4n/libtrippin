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

#ifndef TR_H
#define TR_H
#include <__stdarg_va_list.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// Idk why I added this
#define TR_VERSION "v0.1.0"

// Reference count deez.
typedef struct {
	size_t count;
} TrRefHeader;

// It initializes libtrippin.
void tr_init(const char* log_file);

// It frees libtrippin.
void tr_free(void);

// arenas

// Kilobytes to bytes
#define TR_KB(b) ((b) * 1024)
// Megabytes to bytes
#define TR_MB(b) (TR_KB((b)) * 1024)
// Gigabytes to bytes
#define TR_GB(b) (TR_MB((b)) * 1024)

typedef struct {
	size_t size;
	size_t alloc_pos;
	void* buffer;
} TrArena;

// Makes a new arena :)
TrArena tr_arena_new(size_t size);

// Frees the arena and everything inside it.
void tr_arena_free(TrArena arena);

// Allocates space in the arena.
void* tr_arena_alloc(TrArena arena, size_t size);

// vectors

typedef struct {
	double x;
	double y;
} TrVec2f;

typedef struct {
	int64_t x;
	int64_t y;
} TrVec2i;

typedef struct {
	double x;
	double y;
	double z;
} TrVec3f;

typedef struct {
	int64_t x;
	int64_t y;
	int64_t z;
} TrVec3i;

#define TR_V2_ADD(a, b)  {a.x + b.x, a.y + b.y}
#define TR_V2_SUB(a, b)  {a.x - b.x, a.y - b.y}
#define TR_V2_MUL(a, b)  {a.x * b.x, a.y * b.y}
#define TR_V2_SMUL(a, b) {a.x * b,   a.y * b}
#define TR_V2_DIV(a, b)  {a.x / b.x, a.y / b.y}
#define TR_V2_SDIV(a, b) {a.x / b,   a.y / b}

#define TR_V2_EQ(a, b)   (a.x == b.x && a.y == b.y)
#define TR_V2_NEQ(a, b)  (a.x != b.x && a.y != b.y)
#define TR_V2_LT(a, b)   (a.x < b.x  && a.y < b.y)
#define TR_V2_LTE(a, b)  (a.x <= b.x && a.y <= b.y)
// shout out to lua
#define TR_V2_GT(a, b)   TR_V2_LT(b, a)
#define TR_V2_GTE(a, b)  TR_V2_LTE(b, a)

#define TR_V3_ADD(a, b)  {a.x + b.x, a.y + b.y, a.z + b.z}
#define TR_V3_SUB(a, b)  {a.x - b.x, a.y - b.y, a.z - b.z}
#define TR_V3_MUL(a, b)  {a.x * b.x, a.y * b.y, a.z * b.z}
#define TR_V3_SMUL(a, b) {a.x * b,   a.y * b,   a.z * b}
#define TR_V3_DIV(a, b)  {a.x / b.x, a.y / b.y, a.z / b.z}
#define TR_V3_SDIV(a, b) {a.x / b,   a.y / b,   a.z / b}

#define TR_V3_EQ(a, b)   (a.x == b.x && a.y == b.y && a.z == b.z)
#define TR_V3_NEQ(a, b)  (a.x != b.x && a.y != b.y && a.z != b.z)
#define TR_V3_LT(a, b)   (a.x < b.x  && a.y < b.y  && a.z <  b.z)
#define TR_V3_LTE(a, b)  (a.x <= b.x && a.y <= b.y && a.z <= b.z)
// shout out to lua
#define TR_V3_GT(a, b)   TR_V3_LT(b, a)
#define TR_V3_GTE(a, b)  TR_V3_LTE(b, a)

// logging

#define TR_CONSOLE_COLOR_RESET    "\033[0m"
#define TR_CONSOLE_COLOR_LIB_INFO "\033[0;90m"
#define TR_CONSOLE_COLOR_WARN     "\033[0;93m"
#define TR_CONSOLE_COLOR_ERROR    "\033[0;91m"

typedef enum {
	// literally just for use with raylib
	TR_LOG_LIB_INFO,
	TR_LOG_INFO,
	TR_LOG_WARNING,
	TR_LOG_ERROR,
} TrLogLevel;

// Log without formatting
void tr_print(TrLogLevel level, const char* text);

// Log.
void tr_log(TrLogLevel level, const char* fmt, ...);

// Formatted assert?!!!??!?!??!?1
void tr_assert(bool x, const char* msg, ...);

// uh oh
void tr_panic(const char* msg, ...);

// slices

typedef struct {
	size_t length;
	size_t elem_size;
	void* buffer;
} TrSlice;

// Creates a new slice in an arena. The element size is supposed to be used with sizeof,
// e.g. sizeof(int) for a slice of ints.
TrSlice tr_slice_new(TrArena arena, size_t length, size_t elem_size);

// Gets the element at the specified index. Note this returns a pointer to the element so this is also
// how you change elements.
void* tr_slice_at(TrSlice slice, size_t idx);

// slice but 2d lmao
typedef struct {
	size_t width;
	size_t height;
	size_t elem_size;
	void* buffer;
} TrSlice2D;

// Creates a new 2D slice in an arena. The element size is supposed to be used with sizeof,
// e.g. sizeof(int) for a slice of ints.
TrSlice2D tr_slice2d_new(TrArena arena, size_t width, size_t height, size_t elem_size);

// Gets the element at the specified index. Note this returns a pointer to the element so this is also
// how you change elements.
void* tr_slice2d_at(TrSlice2D slice, size_t x, size_t y);

#ifdef __cplusplus
}
#endif

#endif
