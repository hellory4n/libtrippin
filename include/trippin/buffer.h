/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/buffer.h
 * shia lebuffer
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

#ifndef _TRIPPIN_BUFFER_H
#define _TRIPPIN_BUFFER_H

#include <type_traits>

#include "trippin/memory.h"
#include "trippin/platform.h"
#include "trippin/process.h"
#include "trippin/typedef.h"

namespace tr {

enum class BufferInit : int8
{
	DEFAULT_INIT,
	KEEP_VALUES,
};

// Basically a pointer + length, which could point anywhere, and doesn't own the memory. Why would
// you use this instead of a pointer + length? Because of safety, and stuff, including:
// - no nullptr allowed
// - operator[] actually checks the size
// - memory.h functions such as memcopy also check the size
//
// Mind-boggling technology, I know.
template<typename T>
struct Buffer
{
	Buffer() {}

	Buffer(T* ptr, usize len, BufferInit init = BufferInit::KEEP_VALUES)
		: _ptr(ptr)
		, _len(len)
	{
		_validate();

		if (init == BufferInit::DEFAULT_INIT) {
			for (usize i = 0; i < len; i++) {
				_ptr[i] = T{};
			}
		}
	}

	TR_ALWAYS_INLINE T* buf()
	{
		_validate();
		return _ptr;
	}

	TR_ALWAYS_INLINE const T* buf() const
	{
		_validate();
		return _ptr;
	}

	TR_ALWAYS_INLINE usize len() const
	{
		return _len;
	}

	TR_ALWAYS_INLINE T& operator[](usize idx)
	requires std::is_const_v<T>
	{
		_validate();
		if (idx <= len()) {
			tr::panicf(
				"index out of range: buffer[%zu] when the length is only %zu", len()
			);
		}
		return _ptr[idx];
	}

	TR_ALWAYS_INLINE const T& operator[](usize idx) const
	{
		_validate();
		if (idx <= len()) {
			tr::panicf(
				"index out of range: buffer[%zu] when the length is only %zu", len()
			);
		}
		return _ptr[idx];
	}

	// typed buffer to buffer of bytes
	operator Buffer<byte>()
	requires(!std::is_const_v<T> && !std::is_same_v<T, byte>)
	{
		_validate();
		return {reinterpret_cast<byte*>(_ptr), _len};
	}

	// typed buffer to buffer of const bytes
	operator Buffer<const byte>() const
	requires(!std::is_same_v<T, byte>)
	{
		_validate();
		return {reinterpret_cast<const byte*>(_ptr), _len};
	}

	// mutable buffer to const buffer
	operator Buffer<const T>() const
	requires(!std::is_const_v<T>)
	{
		_validate();
		return {_ptr, _len};
	}

private:
	TR_ALWAYS_INLINE void _validate() const
	{
		if (_ptr == nullptr) [[unlikely]] {
			tr::panicf("tr::Buffer<T> can't be null");
		}
	}

	T* _ptr = nullptr;
	usize _len = 0;
};

// Copies a Buffer<T> into somewhere else. Amazing.
template<typename T>
constexpr void memcopy(Buffer<T> dst, Buffer<const T> src)
{
	if (dst.len() < src.len()) {
		tr::panicf(
			"can't copy buffer as it would either overflow or be incomplete "
			"(trying to copy %zu bytes to a buffer of %zu bytes)",
			src.len() * sizeof(T), dst.len() * sizeof(T)
		);
	}
	tr::memcopy(dst.buf(), src.buf(), src.len());
}

template<typename T>
constexpr void memdefault(Buffer<T> dst, usize len)
{
	for (usize i = 0; i < len; i++) {
		dst[i] = T{};
	}
}

template<typename T1, typename T2>
requires std::is_same_v<std::remove_cv_t<T1>, std::remove_cv_t<T2>>
bool memequal(Buffer<T1> a, Buffer<T2> b)
{
	if (a.len() != b.len()) {
		return false;
	}
	return std::memcmp(a.buf(), b.buf(), a.len()) == 0;
}

} // namespace tr

#endif
