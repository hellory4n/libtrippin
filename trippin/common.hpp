/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/common.hpp
 * Numbers, macros, and utility structs
 *
 * Copyright (C) 2025 by hellory4n <hellory4n@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby
 * granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
 * USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#ifndef _TRIPPIN_COMMON_H
#define _TRIPPIN_COMMON_H

#include <stdint.h>
#include <stddef.h>

#if defined(__GNUC__) || defined(__clang__)
	#define TR_GCC_PRAGMA(X) _Pragma(#X)

	#define TR_GCC_IGNORE_WARNING(Warning) \
		TR_GCC_PRAGMA(GCC diagnostic push) \
		TR_GCC_PRAGMA(GCC diagnostic ignored #Warning)

	#define TR_GCC_RESTORE() TR_GCC_PRAGMA(GCC diagnostic pop)
#else
	#define TR_GCC_IGNORE_WARNING(Warning)
	#define TR_GCC_RESTORE()
#endif

#if (defined(__GNUC__) || defined(__clang__)) && !defined(_WIN32)
	// counting starts at 1 lmao
	#define TR_LOG_FUNC(FmtIdx, VarargsIdx) [[gnu::format(printf, FmtIdx, VarargsIdx)]]
#else
	#define TR_LOG_FUNC(FmtIdx, VarargsIdx)
#endif

#define TR_ARRLEN(T, Array) (sizeof(Array)) / sizeof(T))

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef float float32;
typedef double float64;
typedef size_t usize;
typedef ptrdiff_t isize;

// it's not guaranteed lmao
static_assert(sizeof(usize) == sizeof(isize), "oh no usize and isize aren't the same size");

namespace tr {

// I sure love versions.
static constexpr const char* VERSION = "v2.2.0";

// Initializes the bloody library lmao.
void init();

// Deinitializes the bloody library lmao.
void free();

// including log.hpp here would crash and burn everything :)
TR_LOG_FUNC(1, 2) [[noreturn]] void panic(const char* fmt, ...);

// Like how the spicy modern languages handle null
template<typename T>
struct Maybe
{
private:
	union {
		uint8_t waste_of_space;
		T value;
	};
	bool has_value;

public:
	// Initializes a Maybe<T> as null
	Maybe() : waste_of_space(0), has_value(false) {};

	// Intializes a Maybe<T> with a value
	Maybe(const T& val) : value(val), has_value(true) {};

	Maybe(const Maybe& other) : has_value(other.has_value)
	{
		if (this->has_value) this->value = other.value;
		else this->has_value = 0;
	}

	Maybe& operator=(const Maybe& other)
	{
		if (this != &other) {
			if (this->has_value && other.has_value) {
				this->value = other.value;
			} else if (this->has_value && !other.has_value) {
				this->value.~T();
				this->has_value = false;
			} else if (!this->has_value && other.has_value) {
				this->value = other.value;
				this->has_value = true;
			}
		}
		return *this;
	}

	~Maybe()
	{
		if (this->has_value) {
			value.~T();
		}
	}

	// If true, the maybe is, in fact, a definitely.
	bool is_valid() const
	{
		return this->has_value;
	}

	// Gets the value or panics if it's null
	T& unwrap()
	{
		if (this->has_value) return this->value;
		else tr::panic("couldn't unwrap Maybe<T>");
	}

	// Gets the value or panics if it's null
	const T& unwrap() const
	{
		if (this->has_value) return this->value;
		else tr::panic("couldn't unwrap Maybe<T>");
	}
};

// Functional propaganda
template<typename L, typename R>
struct Either
{
private:
	union {
		L val_left;
		R val_right;
	};
	// False is left, true is right
	bool active;

public:
	Either(L left) : val_left(left), active(false) {};
	Either(R right) : val_right(right), active(true) {};

	~Either()
	{
		if (this->active) {
			this->val_left.~L();
		}
		else {
			this->val_right.~R();
		}
	}

	// If true, it's left. Else, it's right.
	bool is_left() { return this->active; }
	// If true, it's right. Else, it's left.
	bool is_right() { return !this->active; }

	// Returns the left value, or panics if it's not left
	L& left()
	{
		if (!this->active) tr::panic("Either<L, R> is right, not left");
		else return this->val_left;
	}

	// Returns the right value, or panics if it's not right
	R& right()
	{
		if (this->active) tr::panic("Either<L, R> is right, not left");
		else return this->val_right;
	}
};

// It's a pair lmao.
template<typename L, typename R>
struct Pair
{
	L left;
	R right;

	Pair(L left, R right) : left(left), right(right) {};

	~Pair()
	{
		left.~L();
		right.~R();
	}
};

}

#endif
