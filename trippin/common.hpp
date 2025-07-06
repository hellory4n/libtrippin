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
// TODO implement these yourself you scoundrel
#include <variant>
#include <optional>

// TODO there should be msvc versions probably

// is this gcc or clang?
// some warnings are different
#if defined(__clang__)
	#define TR_ONLY_CLANG
#elif defined(__GNUC__)
	#define TR_ONLY_GCC
#endif

// but they're similar enough that we can usually check for both
#ifdef __GNUC__
	#define TR_GCC_OR_CLANG
#endif

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
static constexpr const char* VERSION = "v2.3.0";

// Initializes the bloody library lmao.
void init();

// Deinitializes the bloody library lmao.
void free();

// including log.hpp here would crash and burn everything :)

// Oh god oh fuck. Note this crashes and kills everything, `tr::error` doesn't.
[[noreturn, gnu::format(printf, 1, 2)]] void panic(const char *fmt, ...);

// Functional propaganda
template<typename L, typename R>
struct Either
{
private:
	enum class Side : uint8 { LEFT, RIGHT };

	// TODO implement it yourself you scoundrel
	std::variant<L, R> value;
	Side side;

public:
	Either(const L& left) : value(left), side(Side::LEFT) {}
	Either(const R& right) : value(right), side(Side::RIGHT) {}

	// If true, it's left. Else, it's right.
	bool is_left() const { return this->side == Side::LEFT; }
	// If true, it's right. Else, it's left.
	bool is_right() const { return this->side == Side::RIGHT; }

	// Returns the left value, or panics if it's not left
	L& left() const
	{
		if (!this->is_left()) tr::panic("Either<L, R> is right, not left");
		else return this->value.left;
	}

	// Returns the right value, or panics if it's not right
	R& right() const
	{
		if (!this->is_right()) tr::panic("Either<L, R> is right, not left");
		else return this->value.right;
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
struct Maybe
{
private:
	// TODO implement it yourself you scoundrel
	std::optional<T> value;
	bool has_value = false;

public:
	// Initializes a Maybe<T> as null
	Maybe() : value(), has_value(false) {}

	// Intializes a Maybe<T> with a value
	Maybe(const T& val) : value(val), has_value(true) {}

	// TODO bring this back if it haunts me at some point
	// Maybe(const Maybe& other) : has_value(other.has_value)
	// {
	// 	if (this->has_value) this->value.value = other.value;
	// 	else this->value.waste_of_space = 0;
	// }

	// Maybe& operator=(const Maybe& other)
	// {
	// 	if (this != &other) {
	// 		if (this->has_value && other.has_value) {
	// 			this->value = other.value;
	// 		} else if (this->has_value && !other.has_value) {
	// 			this->value.~T();
	// 			this->has_value = false;
	// 		} else if (!this->has_value && other.has_value) {
	// 			this->value = other.value;
	// 			this->has_value = true;
	// 		}
	// 	}
	// 	return *this;
	// }

	// If true, the maybe is, in fact, a definitely.
	bool is_valid() const
	{
		return this->has_value;
	}

	// Gets the value or panics if it's null
	T& unwrap() const
	{
		if (this->has_value) return this->value.value();
		else tr::panic("couldn't unwrap Maybe<T>");
	}

	// Similar to the `??`/null coalescing operator in modern languages
	T value_or(const T& other) const { return this->is_valid() ? this->unwrap() : other; }

	// Calls a function (usually a lambda) depending on whether it's valid or not.
	void match(std::function<void(T& val)> valid_func, std::function<void()> invalid_func)
	{
		if (this->is_valid()) valid_func(this->unwrap());
		else invalid_func();
	}

	bool operator==(const Maybe& other)
	{
		// TODO is this stupid?
		bool is_valid = this->is_valid() == other.is_valid();
		bool value_eq = is_valid;
		if (this->is_valid() && other.is_valid()) {
			value_eq = this->unwrap() == other.unwrap();
		}
		return is_valid && value_eq;
	}

	bool operator!=(const Maybe& other) { return !(*this == other); }
};

// You're supposed to use this one with references. I fucking love C++.
// TODO there has to be a better way lmao
template<typename T>
struct MaybeRef
{
private:
	// TODO implement it yourself you scoundrel
	std::optional<std::reference_wrapper<T>> value;
	bool has_value = false;

public:
	// Initializes a Maybe<T> as null
	MaybeRef() : value(), has_value(false) {}

	// Intializes a Maybe<T> with a value
	MaybeRef(T& val) : has_value(true)
	{
		this->value = val;
	}

	// TODO bring this back if it haunts me at some point
	// Maybe(const Maybe& other) : has_value(other.has_value)
	// {
	// 	if (this->has_value) this->value.value = other.value;
	// 	else this->value.waste_of_space = 0;
	// }

	// Maybe& operator=(const Maybe& other)
	// {
	// 	if (this != &other) {
	// 		if (this->has_value && other.has_value) {
	// 			this->value = other.value;
	// 		} else if (this->has_value && !other.has_value) {
	// 			this->value.~T();
	// 			this->has_value = false;
	// 		} else if (!this->has_value && other.has_value) {
	// 			this->value = other.value;
	// 			this->has_value = true;
	// 		}
	// 	}
	// 	return *this;
	// }

	// If true, the maybe is, in fact, a definitely.
	bool is_valid() const
	{
		return this->has_value;
	}

	// Gets the value or panics if it's null
	T& unwrap() const
	{
		if (this->has_value) return *this->value.value();
		else tr::panic("couldn't unwrap Maybe<T>");
	}

	// Similar to the `??`/null coalescing operator in modern languages
	T value_or(T& other) const { return this->is_valid() ? this->unwrap() : other; }

	// Calls a function (usually a lambda) depending on whether it's valid or not.
	void match(std::function<void(T& val)> valid_func, std::function<void()> invalid_func)
	{
		if (this->is_valid()) valid_func(this->unwrap());
		else invalid_func();
	}

	bool operator==(const MaybeRef& other)
	{
		// TODO is this stupid?
		bool is_valid = this->is_valid() == other.is_valid();
		bool value_eq = is_valid;
		if (this->is_valid() && other.is_valid()) {
			value_eq = this->unwrap() == other.unwrap();
		}
		return is_valid && value_eq;
	}

	bool operator!=(const MaybeRef& other) { return !(*this == other); }
};

// It's a pair lmao.
template<typename L, typename R>
struct Pair
{
	L left;
	R right;

	Pair(const L& left, const R& right) : left(left), right(right) {}
};

}

#endif
