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
#include <functional>
#include <type_traits> // IWYU pragma: keep

// get compiler
// checking between gcc and clang is useful because some warnings are different
#if defined(__clang__)
	#define TR_ONLY_CLANG
#elif defined(__GNUC__)
	#define TR_ONLY_GCC
#elif defined(_MSC_VER)
	#define TR_ONLY_MSVC
#endif

#if defined(__GNUC__) && defined(_WIN32)
	#define TR_ONLY_MINGW_GCC
#endif

// but they're similar enough that we can usually check for both
#ifdef __GNUC__
	#define TR_GCC_OR_CLANG
#endif

// TODO msvc version
#ifdef TR_GCC_OR_CLANG
	#define TR_GCC_PRAGMA(X) _Pragma(#X)

	#define TR_GCC_IGNORE_WARNING(Warning) \
		TR_GCC_PRAGMA(GCC diagnostic push) \
		TR_GCC_PRAGMA(GCC diagnostic ignored #Warning)

	#define TR_GCC_RESTORE() TR_GCC_PRAGMA(GCC diagnostic pop)
#else
	#define TR_GCC_IGNORE_WARNING(Warning)
	#define TR_GCC_RESTORE()
#endif

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
static constexpr const char* VERSION = "v2.3.3";

// I sure love versions. Format is XYYZZ
static constexpr uint32 VERSION_NUM = 2'03'03;

// Initializes the bloody library lmao.
void init();

// Deinitializes the bloody library lmao.
void free();

// Exits the application. The reason you'd use this instead of libc's `exit()` is because this deinitializes
// crap such as libtrippin (you can add your own functions to run here)
void quit(int32 error_code);

// Adds a function to run when the program quits/panics.
void call_on_quit(std::function<void(void)> func);

// mingw gcc complains about %zu and %li even tho it works fine :)
// TODO this WILL break
#if defined(TR_GCC_OR_CLANG) && !defined(TR_ONLY_MINGW_CC)
[[noreturn, gnu::format(printf, 1, 2)]]
#endif
// i love circular dependencies.
// Oh god oh fuck. Note this crashes and kills everything, `tr::error` doesn't.
void panic(const char* fmt, ...);

// Functional propaganda
template<typename L, typename R>
class Either
{
	enum class Side : uint8 { UNINITIALIZED, LEFT, RIGHT };

	L _left = L();
	R _right = R();
	Side side = Side::UNINITIALIZED;

public:
	Either(const L& left) : _left(left), side(Side::LEFT) {}
	Either(const R& right) : _right(right), side(Side::RIGHT) {}

	// If true, it's left. Else, it's right.
	bool is_left() const { return this->side == Side::LEFT; }
	// If true, it's right. Else, it's left.
	bool is_right() const { return this->side == Side::RIGHT; }

	// Returns the left value, or panics if it's not left
	L& left() const
	{
		if (!this->is_left()) tr::panic("Either<L, R> is right, not left");
		else return const_cast<L&>(this->_left);
	}

	// Returns the right value, or panics if it's not right
	R& right() const
	{
		if (!this->is_right()) tr::panic("Either<L, R> is right, not left");
		else return const_cast<R&>(this->_right);
	}

	// Calls a function (usually a lambda) depending on whether it's left, or right.
	void match(std::function<void(L& left)> left_func, std::function<void(R& right)> right_func)
	{
		if (this->is_left()) left_func(this->left());
		else right_func(this->right());
	}
};

// Like how the spicy modern languages handle null. Note you have to use `MaybeRef<T>` for references,
// because C++.
template<typename T>
class Maybe
{
	// TODO implement it yourself you scoundrel
	Either<T, uint8> value = {};
	bool has_value = false;

public:
	// used for reflection
	using __T = T;

	// Initializes a Maybe<T> as null
	Maybe() : value(0), has_value(false) {}

	// Intializes a Maybe<T> with a value
	Maybe(const T& val) : value(val), has_value(true) {}

	// If true, the maybe is, in fact, a definitely.
	bool is_valid() const
	{
		return this->has_value;
	}

	// Gets the value or panics if it's null
	T& unwrap() const
	{
		if (this->has_value) return const_cast<T&>(this->value.left());
		else tr::panic("couldn't unwrap Maybe<T>");
	}

	// Similar to the `??`/null coalescing operator in modern languages
	const T& value_or(const T& other) const { return this->is_valid() ? this->unwrap() : other; }

	// Calls a function (usually a lambda) depending on whether it's valid or not.
	void match(std::function<void(T& val)> valid_func, std::function<void()> invalid_func)
	{
		if (this->is_valid()) valid_func(this->unwrap());
		else invalid_func();
	}
};

// Like `Maybe<T>`, but for pointers/references. C++ didn't let me do `Maybe<T&>` lmao. This also doesn't
// call destructors, as it's assumed you didn't create the value it's pointing to, if you did, you'd use
// `Maybe<T>`
template<typename T>
class MaybePtr
{
	T* value = nullptr;

public:
	// Intializes a MaybePtr<T> as null
	MaybePtr() : value(nullptr) {}

	// Initializes a MaybePtr<T> with a value.
	MaybePtr(T* val) : value(val) {}

	// Initializes a MaybePtr<T> with a value.
	MaybePtr(T& val) : value(&val) {}

	// If true, the maybe is, in fact, a definitely.
	bool is_valid() const
	{
		return this->value != nullptr;
	}

	// Gets the value as a reference, or panics if it's null
	T& unwrap_ref() const
	{
		if (this->is_valid()) return *this->value;
		else tr::panic("couldn't unwrap MaybePtr<T>");
	}

	// Gets the value as a pointer, or panics if it's null
	T* unwrap_ptr() const
	{
		if (this->is_valid()) return this->value;
		else tr::panic("couldn't unwrap MaybePtr<T>");
	}

	// Similar to the `??`/null coalescing operator in modern languages
	T& value_or(const T& other) const { return this->is_valid() ? this->unwrap_ref() : other; }

	// Similar to the `??`/null coalescing operator in modern languages
	T* value_or(const T* other) const { return this->is_valid() ? this->unwrap_ptr() : other; }

	// Calls a function (usually a lambda) depending on whether it's valid or not.
	void match(std::function<void(T& val)> valid_func, std::function<void()> invalid_func)
	{
		if (this->is_valid()) valid_func(this->unwrap_ref());
		else invalid_func();
	}

	// Calls a function (usually a lambda) depending on whether it's valid or not.
	void match(std::function<void(T* val)> valid_func, std::function<void()> invalid_func)
	{
		if (this->is_valid()) valid_func(this->unwrap_ptr());
		else invalid_func();
	}
};

// It's a pair lmao.
template<typename L, typename R>
struct Pair
{
	L left;
	R right;

	Pair(const L& l, const R& r) : left(l), right(r) {}
};

// Defines bit flag fuckery for enum classes :)
#define TR_BIT_FLAG(T) \
	constexpr T operator|(T lhs, T rhs) \
	{ \
		using N = std::underlying_type_t<T>; \
		return static_cast<T>(static_cast<N>(lhs) | static_cast<N>(rhs)); \
	} \
	constexpr T operator&(T lhs, T rhs) \
	{ \
		using N = std::underlying_type_t<T>; \
		return static_cast<T>(static_cast<N>(lhs) & static_cast<N>(rhs)); \
	} \
	constexpr T operator~(T rhs) \
	{ \
		using N = std::underlying_type_t<T>; \
		return static_cast<T>(~static_cast<N>(rhs)); \
	} \
	constexpr T& operator|=(T& lhs, T rhs) \
	{ \
		lhs = lhs | rhs; \
		return lhs; \
	} \
	constexpr T& operator&=(T& lhs, T rhs) \
	{ \
		lhs = lhs & rhs; \
		return lhs; \
	} \
	/* getting the namespacing right would be too obnoxious so you're supposed to use this */ \
	/* like it's a keyword instead of like `tr::hasflag`/`st::hasflag`/whatever which is */ \
	/* the correct style we use */ \
	constexpr bool hasflag(T value, T flag) \
	{ \
		return (value & flag) == flag; \
	}

}

#endif
