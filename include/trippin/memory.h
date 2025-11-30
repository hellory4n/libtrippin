/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/memory.h
 * Low-level memory utilities
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

#ifndef _TRIPPIN_MEMORY_H
#define _TRIPPIN_MEMORY_H

#include <cstring>
#include <new> // IWYU pragma: keep

#include "trippin/typedef.h"

namespace tr {

template<Number T>
constexpr T kb_to_bytes(T x)
{
	return x * 1024;
}
template<Number T>
constexpr T mb_to_bytes(T x)
{
	return tr::kb_to_bytes(x) * 1024;
}
template<Number T>
constexpr T gb_to_bytes(T x)
{
	return tr::mb_to_bytes(x) * 1024;
}

template<Number T>
constexpr T bytes_to_kb(T x)
{
	return x / 1024;
}
template<Number T>
constexpr T bytes_to_mb(T x)
{
	return tr::bytes_to_kb(x) / 1024;
}
template<Number T>
constexpr T bytes_to_gb(T x)
{
	return tr::bytes_to_mb(x) / 1024;
}

// Allocates memory on the heap, and returns null on failure. You should probably use the higher
// level allocators such as `tr::HeapAlloc`
[[nodiscard, gnu::malloc]]
void* memnew(usize size);

// `tr::memnew<char>(123)` looks better than `(char*)tr::memnew(123)`, I think
template<typename T>
[[nodiscard, gnu::malloc]]
inline T* memnew(usize size)
{
	return as<T*>(tr::memnew(size));
}

// the reference fucks with the type system so we have to use a template :(
void _impl_memfree(void*& ptr);

// Deletes a pointer and sets it to null.
template<typename T>
inline void memfree(T*& ptr)
{
	tr::_impl_memfree((void*&)ptr);
}

// memcpy fucks with some c++ types such as anything with a vtable
template<typename T>
constexpr void memcopy(RefWrapper<T>* dst, const RefWrapper<T>* src, usize len)
{
	for (usize i = 0; i < len; i++) {
		if constexpr (std::is_reference_v<T> || std::is_trivially_copyable_v<T>) {
			dst[i] = src[i];
		}
		else {
			new (&dst[i]) T(src[i]);
		}
	}
}

// memcpy fucks with some c++ types such as anything with a vtable
template<typename T>
constexpr void memcopy(T* dst, const T* src, usize len)
{
	for (usize i = 0; i < len; i++) {
		// hopefully the compiler can optimize this
		new (&dst[i]) T(src[i]);
	}
}

// `memset` can be optimized away, this can't (hopefully)
inline void memmagic(void* dst, usize len, byte val)
{
	volatile byte* ptr = (volatile byte*)dst;
	for (usize i = 0; i < len; i++) {
		ptr[i] = val;
	}
}

// Sets all the items in a buffer to its default value (which may be 0 or something else entirely)
template<typename T>
constexpr void memreset(T* dst, usize len)
{
	for (usize i = 0; i < len; i++) {
		dst[i] = T{};
	}
}

// Returns true if the contents of 2 buffers are equal
inline bool memequal(const void* buf1, usize len1, const void* buf2, usize len2)
{
	if (len1 != len2) {
		return false;
	}
	return std::memcmp(buf1, buf2, len1) == 0;
}

} // namespace tr

#endif
