/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/array.h
 * Safe stack/static array
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

#ifndef _TRIPPIN_ARRAY_H
#define _TRIPPIN_ARRAY_H

#include <initializer_list>

#include "trippin/buffer.h"
#include "trippin/math.h"
#include "trippin/memory.h"
#include "trippin/process.h"
#include "trippin/typedef.h"

namespace tr {

// A safe stack/static array, much like `std::array`.
template<usize N, typename T>
struct Array
{
	constexpr Array() {}

	constexpr Array(std::initializer_list<T> initlist)
	{
		// TODO should this panic if the initlist is bigger?
		tr::memcopy(_array, initlist.begin(), tr::min(N, initlist.size()));
	}

	// make an array from another array why the fuck not
	template<usize N2>
	constexpr Array(T (&arr)[N2])
	{
		tr::memcopy(_array, arr, tr::min(N, N2));
	}

	constexpr T* buf() const
	{
		return _array;
	}

	constexpr usize len() const
	{
		return N;
	}

	template<std::integral NType>
	constexpr T& operator[](NType idx)
	{
		if (idx < 0 || idx >= as<isize>(len())) {
			tr::panicf(
				"index out of range: array[%li] when the length is only %zu",
				as<int64>(idx), len()
			);
		}
		return _array[idx];
	}

	template<std::integral NType>
	constexpr const T& operator[](NType idx) const
	{
		if (idx < 0 || idx >= as<isize>(len())) {
			tr::panicf(
				"index out of range: array[%li] when the length is only %zu",
				as<int64>(idx), len()
			);
		}
		return _array[idx];
	}

	operator Buffer<const T>() const
	{
		return Buffer<const T>{_array};
	}

	operator Buffer<T>()
	{
		return Buffer<T>{_array};
	}

	constexpr bool is_empty() const
	{
		return len() == 0;
	}

	constexpr T& first() const
	{
		return (*this)[0];
	}

	constexpr T& last() const
	{
		return (*this)[len() - 1];
	}

private:
	T _array[N] = {};
};

} // namespace tr

#endif
