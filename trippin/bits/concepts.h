/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/bits/concepts.h
 * Concepts are a C++20 feature
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

#ifndef _TRIPPIN_BITS_CONCEPTS_H
#define _TRIPPIN_BITS_CONCEPTS_H

#include <type_traits>

namespace tr {

// nubmerb :D
template<typename T>
concept Number = std::is_integral_v<T> || std::is_floating_point_v<T>;

template<typename T>
concept Character =
	std::is_same_v<T, char> || std::is_same_v<T, wchar_t> || std::is_same_v<T, char8_t> ||
	std::is_same_v<T, char16_t> || std::is_same_v<T, char32_t>;

// not a concept idc

// someone at C++ hq decided for some fucking reason that actually, a reference is not just a fancy
// pointer...stupid i know
template<typename T>
using RefWrapper = std::conditional_t<std::is_reference_v<T>, std::remove_reference_t<T>*, T>;

}

#endif
