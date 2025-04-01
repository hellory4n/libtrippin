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
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Idk why I added this
#define TRIPPIN_VERSION "v0.1.0"

// I love exploiting the compiler
#define TRIPPIN_DESTRUCTOR(func) __attribute__((cleanup(func)))

// reference counting

// Default destructor thingy
#define TRIPPIN_REF TRIPPIN_DESTRUCTOR(trippin_release)

#ifndef TRIPPIN_NO_SHORTHAND
#define tref TRIPPIN_REF
#define tnew(type) trippin_new(sizeof(type))
#define tnil NULL
#define tpass(var) trippin_reference(var)
#endif

// Reference count deez.
typedef struct {
	size_t count;
} TrippinRefHeader;

// Literally just malloc with a check
void* trippin_new(size_t size);

// Similar to just passing the pointer, except this also increases the reference count.
void* trippin_reference(void* ptr);

// Decreases the reference count, and frees the data.
void trippin_release(void* ptr);

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

#ifdef __cplusplus
}
#endif

#endif
