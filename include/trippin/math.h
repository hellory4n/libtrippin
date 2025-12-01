/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/math.h
 * Basic math functions
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

#ifndef _TRIPPIN_MATH_H
#define _TRIPPIN_MATH_H

#include <cmath> // IWYU pragma: export

namespace tr {

// to make the compiler happy
template<typename T>
constexpr T min(T a)
{
	return a;
}

// Picks the smallest of a set of numbers
template<typename T, typename... Ts>
constexpr T min(T a, Ts... args)
{
	T b = min(args...);
	return (a < b ? a : b);
}

// to make the compiler happy
template<typename T>
constexpr T max(T a)
{
	return a;
}

// Picks the smallest of a set of numbers
template<typename T, typename... Ts>
constexpr T max(T a, Ts... args)
{
	T b = max(args...);
	return (a > b ? a : b);
}

// clamp
template<typename T>
constexpr T clamp(T n, T min, T max)
{
	if (n < min) {
		n = min;
	}
	if (n > max) {
		n = max;
	}
	return max;
}

} // namespace tr

#endif
