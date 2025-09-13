/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/common.h
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

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <type_traits>
#include <utility>

// check for C++17 support
#ifndef _MSC_VER
	#if __cplusplus < 201703L
		#error "libtrippin requires C++17 or higher"
	#endif
#else
	// as always msvc is a bitch and doesn't correctly define __cplusplus until newer versions,
	// but only if you use some extra flag of course
	// like why
	// what the fuck is wrong with you
	#if _MSC_VER >= 1914
		#if _MSVC_LANG < 201703L
			#error "libtrippin requires C++17 or higher"
		#endif
	#else
		#error "Your Visual Studio installation is too old, please update to Visual Studio 2017 or higher"
	#endif
#endif

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

	/* so you don't have to check between GCC and clang, it'll just shut up */
	#ifdef TR_ONLY_CLANG
		#define TR_GCC_IGNORE_WARNING(Warning) \
			TR_GCC_PRAGMA(GCC diagnostic push)                  \
			TR_GCC_PRAGMA(GCC diagnostic ignored "-Wunknown-warning-option")                  \
			TR_GCC_PRAGMA(GCC diagnostic push)                  \
			TR_GCC_PRAGMA(GCC diagnostic ignored #Warning)
	#else
		#define TR_GCC_IGNORE_WARNING(Warning) \
			TR_GCC_PRAGMA(GCC diagnostic push)                  \
			TR_GCC_PRAGMA(GCC diagnostic ignored "-Wpragmas")                  \
			TR_GCC_PRAGMA(GCC diagnostic push)                  \
			TR_GCC_PRAGMA(GCC diagnostic ignored #Warning)
	#endif

	#define TR_GCC_RESTORE() \
		TR_GCC_PRAGMA(GCC diagnostic pop)    \
		TR_GCC_PRAGMA(GCC diagnostic pop)
#else
	#define TR_GCC_PRAGMA(X)
	#define TR_GCC_IGNORE_WARNING(Warning)
	#define TR_GCC_RESTORE()
#endif

// evil macro fuckery
#define _TR_CONCAT2(A, B) A##B
#define _TR_CONCAT(A, B) _TR_CONCAT2(A, B)
#define _TR_UNIQUE_NAME(Base) _TR_CONCAT(Base, __LINE__)

// number types
// i'm not a huge fan of random '_t's everywhere
using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;

using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

using float32 = float;
using float64 = double;

using usize = size_t;
using isize = ptrdiff_t;

// it's usually true, but not guaranteed by the standard
static_assert(sizeof(usize) == sizeof(isize), "size_t and ptrdiff_t must be the same size");
static_assert(sizeof(float32) == 4, "float must be 32-bits");
static_assert(sizeof(float64) == 8, "double must be 64-bits");

namespace tr {

// I sure love versions.
static constexpr const char* VERSION = "v2.6.0";

// I sure love versions. Format is XYYZZ
static constexpr uint32 VERSION_NUM = 2'06'00;
static constexpr uint32 VERSION_MAJOR = 2;
static constexpr uint32 VERSION_MINOR = 6;
static constexpr uint32 VERSION_PATCH = 0;

// Initializes the bloody library lmao.
void init();

// Deinitializes the bloody library lmao.
void free();

// Exits the application. The reason you'd use this instead of libc's `exit()` is because this
// deinitializes crap such as libtrippin (you can add your own functions to run here)
[[noreturn]]
void quit(int32 error_code);

// Adds a function to run when the program quits/panics.
void call_on_quit(std::function<void(bool is_panic)> func);

// mingw gcc complains about %zu and %li even tho it works fine
// TODO this WILL break
#if defined(TR_GCC_OR_CLANG) && !defined(TR_ONLY_MINGW_GCC)
[[gnu::format(printf, 1, 2)]]
#endif
// i love circular dependencies.
// Oh god oh fuck. Note this crashes and kills everything, `tr::error` doesn't.
[[noreturn]]
void panic(const char* fmt, ...);

// someone at C++ hq decided for some fucking reason that actually, a reference is not just a fancy
// pointer...stupid i know
template<typename T>
using RefWrapper = std::conditional_t<
	std::is_reference_v<T>,
	std::remove_reference_t<T>*, // store pointer for references
	std::remove_cv_t<std::remove_reference_t<T>> // store decayed value for non-refs
	>;

// Functional propaganda
template<typename L, typename R>
class Either
{
	enum class Side : uint8
	{
		UNINITIALIZED,
		LEFT,
		RIGHT
	};

	union {
		RefWrapper<L> _left;
		RefWrapper<R> _right;
	};
	Side side = Side::UNINITIALIZED;

public:
	using LeftType = L;
	using RightType = R;

	Either()
		: side(Side::UNINITIALIZED)
	{
	}

	Either(L l)
		: side(Side::LEFT)
	{
		// c++ is consuming my brain
		// it's quite sad i hope they find a cure
		if constexpr (std::is_reference_v<L>) {
			this->_left = &l;
		}
		else {
			new (&this->_left) L(std::forward<L>(l));
		}
	}

	Either(R r)
		: side(Side::RIGHT)
	{
		if constexpr (std::is_reference_v<R>) {
			this->_right = &r;
		}
		else {
			new (&this->_left) R(std::forward<R>(r));
		}
	}

	void free()
	{
		// TODO this WILL break
		if constexpr (std::is_pointer_v<L> || std::is_pointer_v<R>) {
			return;
		}

		if (this->side == Side::LEFT) {
			if constexpr (!std::is_reference_v<L>) {
				using U = std::remove_cv_t<L>;
				reinterpret_cast<U*>(&this->_left)->~U();
			}
		}
		else {
			if constexpr (!std::is_reference_v<R>) {
				using U = std::remove_cv_t<R>;
				reinterpret_cast<U*>(&this->_right)->~U();
			}
		}
	}

	// evil rule of 3 fuckery
	~Either()
	{
		this->free();
	}

	Either(const Either& other)
		: side(other.side)
	{
		if (this->side == Side::LEFT) {
			if constexpr (std::is_reference_v<L>) {
				this->_left = other._left;
			}
			else {
				new (&this->_left) L(*reinterpret_cast<const L*>(&other._left));
			}
		}
		else {
			if constexpr (std::is_reference_v<R>) {
				this->_right = other._right;
			}
			else {
				new (&this->_right) R(*reinterpret_cast<const R*>(&other._right));
			}
		}
	}

	Either& operator=(const Either& other)
	{
		if (this != &other) {
			this->free();
			new (this) Either(other);
		}
		return *this;
	}

	// If true, it's left. Else, it's right.
	bool is_left() const
	{
		return this->side == Side::LEFT;
	}
	// If true, it's right. Else, it's left.
	bool is_right() const
	{
		return this->side == Side::RIGHT;
	}

	// Returns the left value, or panics if it's not left
	L left() const
	{
		if (!this->is_left()) {
			tr::panic("tr::Either<L, R> is right, NOT left");
		}

		if constexpr (std::is_reference_v<L>) {
			return *this->_left;
		}
		else {
			return *reinterpret_cast<const L*>(&this->_left);
		}
	}

	// Returns the right value, or panics if it's not right
	R right() const
	{
		if (!this->is_right()) {
			tr::panic("tr::Either<L, R> is right, NOT left");
		}

		if constexpr (std::is_reference_v<R>) {
			return *this->_right;
		}
		else {
			return *reinterpret_cast<const R*>(&this->_right);
		}
	}

	// Calls a function (usually a lambda) depending on wheth1er it's left, or right.
	void match(std::function<void(L left)> left_func, std::function<void(R right)> right_func)
	{
		if (this->is_left()) {
			left_func(this->left());
		}
		else {
			right_func(this->right());
		}
	}
};

// Like how the spicy modern languages handle null.
template<typename T>
class Maybe
{
	Either<T, uint8> value = {};
	bool has_value = false;

public:
	using Type = T;

	// Initializes a Maybe<T> as null
	Maybe()
		: value(0)
	{
	}

	// Intializes a Maybe<T> with a value
	Maybe(T val)
		: value(val)
		, has_value(true)
	{
	}

	// If true, the maybe is, in fact, a definitely.
	bool is_valid() const
	{
		return this->has_value;
	}

	// Gets the value or panics if it's null
	T unwrap() const
	{
		if (this->has_value) {
			return this->value.left();
		}
		tr::panic("couldn't unwrap Maybe<T>");
	}

	// Similar to the `??`/null coalescing operator in modern languages
	const T value_or(const T other) const
	{
		return this->is_valid() ? this->unwrap() : other;
	}

	// Calls a function (usually a lambda) depending on whether it's valid or not.
	void match(std::function<void(T val)> valid_func, std::function<void()> invalid_func)
	{
		if (this->is_valid()) {
			valid_func(this->unwrap());
		}
		else {
			invalid_func();
		}
	}
};

// Like `Maybe<T>`, but for pointers/references. C++ didn't let me do `Maybe<T&>` lmao. This also
// doesn't call destructors, as it's assumed you didn't create the value it's pointing to, if you
// did, you'd use `Maybe<T>`
template<typename T>
// WHY BJANRE STROUSPTRUP WHY
class [[deprecated("you can do tr::Maybe<T&> now lmao")]] MaybePtr
{
	T* value = nullptr;

public:
	using Type = T;

	// Intializes a MaybePtr<T> as null
	MaybePtr()
		: value(nullptr)
	{
	}

	// Initializes a MaybePtr<T> with a value.
	MaybePtr(T* val)
		: value(val)
	{
	}

	// Initializes a MaybePtr<T> with a value.
	MaybePtr(T& val)
		: value(&val)
	{
	}

	// If true, the maybe is, in fact, a definitely.
	bool is_valid() const
	{
		return this->value != nullptr;
	}

	// Gets the value as a reference, or panics if it's null
	T& unwrap_ref() const
	{
		if (this->is_valid()) {
			return *this->value;
		}
		tr::panic("couldn't unwrap MaybePtr<T>");
	}

	// Gets the value as a pointer, or panics if it's null
	T* unwrap_ptr() const
	{
		if (this->is_valid()) {
			return this->value;
		}
		tr::panic("couldn't unwrap MaybePtr<T>");
	}

	// Similar to the `??`/null coalescing operator in modern languages
	T& value_or(const T& other) const
	{
		return this->is_valid() ? this->unwrap_ref() : other;
	}

	// Similar to the `??`/null coalescing operator in modern languages
	T* value_or(const T* other) const
	{
		return this->is_valid() ? this->unwrap_ptr() : other;
	}

	// Calls a function (usually a lambda) depending on whether it's valid or not.
	void match(std::function<void(T& val)> valid_func, std::function<void()> invalid_func)
	{
		if (this->is_valid()) {
			valid_func(this->unwrap_ref());
		}
		else {
			invalid_func();
		}
	}

	// Calls a function (usually a lambda) depending on whether it's valid or not.
	void match(std::function<void(T* val)> valid_func, std::function<void()> invalid_func)
	{
		if (this->is_valid()) {
			valid_func(this->unwrap_ptr());
		}
		else {
			invalid_func();
		}
	}
};

// It's a pair lmao.
template<typename L, typename R>
struct Pair
{
	L left;
	R right;

	using LeftType = L;
	using RightType = R;

	Pair(L l, R r)
		: left(l)
		, right(r)
	{
	}

	// TODO do destructors work?
	// destructors suck
};

// Defines bit flag fuckery for enum classes :)
#define TR_BIT_FLAG(T)                                                                            \
	constexpr T operator|(T lhs, T rhs)                                                       \
	{                                                                                         \
		using N = std::underlying_type_t<T>;                                              \
		return static_cast<T>(static_cast<N>(lhs) | static_cast<N>(rhs));                 \
	}                                                                                         \
	constexpr T operator&(T lhs, T rhs)                                                       \
	{                                                                                         \
		using N = std::underlying_type_t<T>;                                              \
		return static_cast<T>(static_cast<N>(lhs) & static_cast<N>(rhs));                 \
	}                                                                                         \
	constexpr T operator~(T rhs)                                                              \
	{                                                                                         \
		using N = std::underlying_type_t<T>;                                              \
		return static_cast<T>(~static_cast<N>(rhs));                                      \
	}                                                                                         \
	constexpr T& operator|=(T& lhs, T rhs)                                                    \
	{                                                                                         \
		lhs = lhs | rhs;                                                                  \
		return lhs;                                                                       \
	}                                                                                         \
	constexpr T& operator&=(T& lhs, T rhs)                                                    \
	{                                                                                         \
		lhs = lhs & rhs;                                                                  \
		return lhs;                                                                       \
	}                                                                                         \
	/* getting the namespacing right would be too obnoxious so you're supposed to use this */ \
	/* like it's a keyword instead of like `tr::hasflag`/`st::hasflag`/whatever which is */   \
	/* the correct style we use */                                                            \
	constexpr bool hasflag(T value, T flag)                                                   \
	{                                                                                         \
		return (value & flag) == flag;                                                    \
	}

// I love reinventing the wheel
// TODO this kinda sucks
template<typename T>
class RangeIterator
{
	T start;
	T stop;
	T current;
	// TODO what happens if step is negative?
	T step;

public:
	// DIE
	TR_GCC_IGNORE_WARNING(-Wshadow)
	explicit RangeIterator(T start, T end, T cur, T step)
		: start(start)
		, stop(end)
		, current(cur)
		, step(step)
	{
	}
	TR_GCC_RESTORE()

	const T& operator*() const
	{
		return this->current;
	}

	RangeIterator& operator++()
	{
		// TODO could be checked once for a nano-optimization
		if (this->start < this->stop) {
			this->current += this->step;
		}
		else {
			this->current -= this->step;
		}
		return *this;
	}

	bool operator!=(const RangeIterator& rhs) const
	{
		if (this->start < this->stop) {
			return this->current < rhs.stop;
		}
		return this->current > rhs.stop;
	}

	RangeIterator begin()
	{
		return *this;
	}

	RangeIterator end()
	{
		return *this;
	}
};

// Shorthand for a C-style loop. Similar to Go's `range()`
template<typename T>
RangeIterator<T> range(T start, T end, T step)
{
	return RangeIterator<T>(start, end, start, step);
}

// Shorthand for a C-style loop. Similar to Go's `range()`
template<typename T>
RangeIterator<T> range(T start, T end)
{
	return RangeIterator<T>(start, end, start, 1);
}

// Shorthand for a C-style loop. Similar to Go's `range()`
template<typename T>
RangeIterator<T> range(T end)
{
	return RangeIterator<T>(0, end, 0, 1);
}

// defer
// usage: e.g. TR_DEFER(free(ptr));
template<typename Fn>
struct _Defer
{
	Fn fn;

	_Defer(Fn fn)
		: fn(fn)
	{
	}

	~_Defer()
	{
		fn();
	}
};

template<typename Fn>
_Defer<Fn> _defer_func(Fn fn)
{
	return _Defer<Fn>(fn);
}

#define TR_DEFER(...) auto _TR_UNIQUE_NAME(_tr_defer) = tr::_defer_func([&]() { __VA_ARGS__; })

}

#endif
