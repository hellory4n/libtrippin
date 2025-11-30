/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/typedef.h
 * Number typedefs, concepts, and stuff
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

#ifndef _TRIPPIN_TYPEDEF_H
#define _TRIPPIN_TYPEDEF_H

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

#include "trippin/platform.h"

namespace tr {

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

// A byte makes it clear you're working with bytes (e.g. low level faffery), while an uint8 may
// just be an unsigned int that happens to be 8 bits for whatever reason
using byte = uint8_t;

// it's usually true, but not guaranteed by the standard
// TODO c++23 has float32_t, float64_t, and even float16_t
static_assert(sizeof(usize) == sizeof(isize), "size_t and ptrdiff_t must be the same size");
static_assert(sizeof(float32) == 4, "float must be 32-bits");
static_assert(sizeof(float64) == 8, "double must be 64-bits");

// It's a number duh
template<typename T>
concept Number = std::is_integral_v<T> || std::is_floating_point_v<T>;

// C++'s type system refuses to treat a reference as a fancy pointer
template<typename T>
struct RefWrapper
{
	using WrapT = std::conditional_t<std::is_reference_v<T>, std::remove_reference_t<T>*, T>;
	WrapT val{};

	TR_ALWAYS_INLINE RefWrapper(T v)
	requires(!std::is_reference_v<T>)
		: val(v)
	{
	}

	TR_ALWAYS_INLINE RefWrapper(T v)
	requires(std::is_reference_v<T>)
		: val(&v)
	{
	}

	TR_ALWAYS_INLINE operator T()
	{
		if constexpr (std::is_reference_v<T>) {
			return *val;
		}
		else {
			return val;
		}
	}

	TR_ALWAYS_INLINE operator const T() const
	{
		if constexpr (std::is_reference_v<T>) {
			return *val;
		}
		else {
			return val;
		}
	}

	TR_ALWAYS_INLINE operator WrapT*()
	requires std::is_reference_v<T>
	{
		return val;
	}

	TR_ALWAYS_INLINE operator const WrapT*() const
	requires std::is_reference_v<T>
	{
		return val;
	}
};

// the casts have ridiculous names and i don't care what you think
// cmon literally every other language on this beautiful blue planet has a shorter cast
template<typename To, typename From>
[[nodiscard]]
constexpr To as(From val)
{
	return static_cast<To>(std::forward<From>(val));
}

// im not scared of the computer i will reinterpret all over the place
template<typename To, typename From>
[[nodiscard]]
TR_ALWAYS_INLINE To reinterpret(From val)
{
	return reinterpret_cast<To>(std::forward<From>(val));
}

// dynamic_cast isn't very common and const_cast is evil so they're not included here

// version crap
constexpr const char* VERSION_STR = "v3.0.0-dev";
// Format is XYYZZ
constexpr uint32 VERSION_NUM = 3'00'00;
constexpr uint32 VERSION_MAJOR = 3;
constexpr uint32 VERSION_MINOR = 0;
constexpr uint32 VERSION_PATCH = 0;

} // namespace tr

// tr:: even for numbers is a bit much imo tbh ong frfr
#ifndef TR_NO_USING_TYPEDEFS
using tr::as;
using tr::byte;
using tr::char16;
using tr::char32;
using tr::char8;
using tr::float32;
using tr::float64;
using tr::int16;
using tr::int32;
using tr::int64;
using tr::int8;
using tr::isize;
using tr::reinterpret;
using tr::uint16;
using tr::uint32;
using tr::uint64;
using tr::uint8;
using tr::usize;
#endif

#endif
