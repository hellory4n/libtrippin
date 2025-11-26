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

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstring>
// TODO this includes <vector>, <array>, and <unordered_map> for some reason (not ideal)
#include <functional>
#include <type_traits>
#include <utility>

#include "trippin/bits/concepts.h" // IWYU pragma: export
#include "trippin/bits/macros.h" // IWYU pragma: export
#include "trippin/bits/platform.h" // IWYU pragma: export

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

using char8 = char8_t;
using char16 = char16_t;
using char32 = char32_t;

// a byte makes it clear you're working with bytes (e.g. low level faffery), while an uint8 may just
// be an unsigned int that happens to be 8 bits for whatever reason
using byte = uint8_t;

// it's usually true, but not guaranteed by the standard
// TODO c++23 has float32_t, float64_t, and even float16_t
static_assert(sizeof(usize) == sizeof(isize), "size_t and ptrdiff_t must be the same size");
static_assert(sizeof(float32) == 4, "float must be 32-bits");
static_assert(sizeof(float64) == 8, "double must be 64-bits");

namespace tr {

// I sure love versions.
constexpr const char* VERSION = "v2.8.0";

// I sure love versions. Format is XYYZZ
constexpr uint32 VERSION_NUM = 2'08'00;
constexpr uint32 VERSION_MAJOR = 2;
constexpr uint32 VERSION_MINOR = 8;
constexpr uint32 VERSION_PATCH = 0;

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
[[noreturn]]
void panic_args(const char* fmt, va_list args);

// Functional propaganda
template<typename L, typename R>
class [[deprecated("making Either<L, R> not be horrible isn't worth it")]] Either
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

	constexpr Either()
		: side(Side::UNINITIALIZED)
	{
	}

	constexpr Either(L l)
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

	constexpr Either(R r)
		: side(Side::RIGHT)
	{
		if constexpr (std::is_reference_v<R>) {
			this->_right = &r;
		}
		else {
			new (&this->_left) R(std::forward<R>(r));
		}
	}

	constexpr void free()
	{
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

	// evil rule of 3 fuckery otherwise c++ kills me to death
	constexpr ~Either()
	{
		this->free();
	}

	constexpr Either(const Either& other)
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

	constexpr Either& operator=(const Either& other)
	{
		if (this != &other) {
			this->free();
			new (this) Either(other);
		}
		return *this;
	}

	// If true, it's left. Else, it's right.
	constexpr bool is_left() const
	{
		return this->side == Side::LEFT;
	}
	// If true, it's right. Else, it's left.
	constexpr bool is_right() const
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
	constexpr void
	match(const std::function<void(L left)>& on_left,
	      const std::function<void(R right)>& on_right)
	{
		if (this->is_left()) {
			on_left(this->left());
		}
		else {
			on_right(this->right());
		}
	}
};

// Like how the spicy modern languages handle null.
template<typename T>
class Maybe
{
	RefWrapper<T> _value{};
	bool _has_value = false;

public:
	using Type = T;

	// Initializes a Maybe<T> as null
	constexpr Maybe()
		: _has_value(false)
	{
	}

	// Intializes a Maybe<T> with a value
	constexpr Maybe(T val)
		: _has_value(true)
	{
		if constexpr (std::is_reference_v<T>) {
			_value = &val;
		}
		else {
			_value = val;
		}
	}

	// If true, the maybe is, in fact, a definitely.
	constexpr bool is_valid() const
	{
		return _has_value;
	}

	// If true, the maybe is, in fact, a nope.
	constexpr bool is_invalid() const
	{
		return !_has_value;
	}

	// Gets the value or panics if it's null
	constexpr T unwrap() const
	{
		if (_has_value) {
			if constexpr (std::is_reference_v<T>) {
				return *_value;
			}
			else {
				return _value;
			}
		}
		tr::panic("couldn't unwrap Maybe<T>");
	}

	// Just unwrap() but with an error message (and formatted too, how fancy)
	T expect(const char* fmt, ...) const
	{
		va_list arg;
		va_start(arg, fmt);
		if (_has_value) {
			return _value;
		}
		tr::panic_args(fmt, arg);
		va_end(arg);
	}

	constexpr T operator*() const
	{
		return unwrap();
	}

	// Similar to the `??`/null coalescing operator in modern languages
	constexpr const T value_or(const T other) const
	{
		return is_valid() ? unwrap() : other;
	}

	// Calls a function (usually a lambda) depending on whether it's valid or not.
	constexpr void
	match(const std::function<void(T val)>& on_valid, const std::function<void()>& on_invalid)
	{
		if (is_valid()) {
			on_valid(unwrap());
		}
		else {
			on_invalid();
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

// 'Macro argument should be enclosed in parentheses'
// it's a fucking type
// NOLINTBEGIN(bugprone-macro-parentheses)

// Defines bit flag fuckery for enum classes :)
#define TR_BIT_FLAG(T)                                                            \
	constexpr T operator|(T lhs, T rhs)                                       \
	{                                                                         \
		using N = std::underlying_type_t<T>;                              \
		return static_cast<T>(static_cast<N>(lhs) | static_cast<N>(rhs)); \
	}                                                                         \
	constexpr T operator&(T lhs, T rhs)                                       \
	{                                                                         \
		using N = std::underlying_type_t<T>;                              \
		return static_cast<T>(static_cast<N>(lhs) & static_cast<N>(rhs)); \
	}                                                                         \
	constexpr T operator~(T rhs)                                              \
	{                                                                         \
		using N = std::underlying_type_t<T>;                              \
		return static_cast<T>(~static_cast<N>(rhs));                      \
	}                                                                         \
	constexpr T& operator|=(T& lhs, T rhs)                                    \
	{                                                                         \
		lhs = lhs | rhs;                                                  \
		return lhs;                                                       \
	}                                                                         \
	constexpr T& operator&=(T& lhs, T rhs)                                    \
	{                                                                         \
		lhs = lhs & rhs;                                                  \
		return lhs;                                                       \
	}                                                                         \
	constexpr bool hasflag(T value, T flag)                                   \
	{                                                                         \
		return (value & flag) == flag;                                    \
	}

// NOLINTEND(bugprone-macro-parentheses)

// I love reinventing the wheel
// TODO this kinda sucks
template<Number T>
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
template<Number T>
[[deprecated("this sucks just use a for loop")]]
RangeIterator<T> range(T start, T end, T step)
{
	return RangeIterator<T>(start, end, start, step);
}

// Shorthand for a C-style loop. Similar to Go's `range()`
template<Number T>
[[deprecated("this sucks just use a for loop")]]
RangeIterator<T> range(T start, T end)
{
	return RangeIterator<T>(start, end, start, 1);
}

// Shorthand for a C-style loop. Similar to Go's `range()`
template<Number T>
[[deprecated("this sucks just use a for loop")]]
RangeIterator<T> range(T end)
{
	return RangeIterator<T>(0, end, 0, 1);
}

// So you can check for debug without having to do ugly preprocessing fuckery :)
constexpr bool is_debug()
{
#if defined(_DEBUG) || defined(DEBUG)
	return true;
#elif defined(NDEBUG)
	return false;
// no debug macros defined, assume release
#else
	return false;
#endif
}

}

#endif
