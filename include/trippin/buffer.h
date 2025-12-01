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

#include <concepts>
#include <type_traits>

#include "trippin/memory.h"
#include "trippin/platform.h"
#include "trippin/process.h"
#include "trippin/typedef.h"

namespace tr {

enum class BufferInit : uint8
{
	DEFAULT_INIT,
	KEEP_VALUES,
};

// Basically a pointer + length, which could point anywhere, and doesn't own the memory. Why would
// you use this instead of a pointer + length? Because of safety, and stuff, including:
// - no nullptr allowed
// - opt-out instead of opt-in default initialization
// - operator[] actually checks the size
// - memory.h functions such as memcopy also check the size
// - signed integer support so that it's easier to find an overflow
//
// Mind-boggling technology, I know. It doesn't make such bugs impossible, but it does make creating
// them much harder and fixing them much easier. It's not that much overhead either, or at least for
// any CPU made in the last decade.
template<typename T>
struct Buffer
{
	constexpr Buffer() {}

	template<std::integral NType>
	constexpr Buffer(T* ptr, NType len, BufferInit init)
	requires(!std::is_const_v<T>)
		: _ptr(ptr)
	{
		_validate();
		if (len < 0) {
			tr::panicf("buffer length can't be negative (expected >=0, got %zu", len);
		}
		else {
			_len = as<usize>(len);
		}

		if (init == BufferInit::DEFAULT_INIT) {
			for (usize i = 0; i < len; i++) {
				_ptr[i] = T{};
			}
		}
	}

	template<std::integral NType>
	constexpr Buffer(T* ptr, NType len)
		: _ptr(ptr)
	{
		_validate();
		if (len < 0) {
			tr::panicf("buffer length can't be negative (expected >=0, got %zu", len);
		}
		else {
			_len = as<usize>(len);
		}
	}

	// mild fuckery so that you can pass a static array
	template<usize N>
	constexpr Buffer(T (&arr)[N])
		: _ptr(arr)
		, _len(N)
	{
	}

	TR_ALWAYS_INLINE T* buf() const
	{
		_validate();
		return _ptr;
	}

	TR_ALWAYS_INLINE usize len() const
	{
		return _len;
	}

	template<std::integral NType>
	TR_ALWAYS_INLINE T& operator[](NType idx) const
	{
		_validate();
		if (idx < 0 || idx >= len()) {
			tr::panicf(
				"index out of range: buffer[%zu] when the length is only %zu", idx,
				len()
			);
		}
		return _ptr[idx];
	}

	// typed buffer to buffer of bytes
	// TODO should this be explicit? idk
	operator Buffer<byte>()
	requires(!std::is_const_v<T> && !std::is_same_v<T, byte>)
	{
		_validate();
		return {reinterpret<byte*>(_ptr), _len};
	}

	// typed buffer to buffer of const bytes
	operator Buffer<const byte>() const
	requires(!std::is_same_v<T, byte>)
	{
		_validate();
		return {reinterpret<const byte*>(_ptr), _len};
	}

	// utility functions
	constexpr bool is_empty() const
	{
		return _len == 0;
	}

	TR_ALWAYS_INLINE T& first() const
	{
		return (*this)[0];
	}

	TR_ALWAYS_INLINE T& last() const
	{
		return (*this)[len() - 1];
	}

	// Returns a smaller part of a buffer
	TR_ALWAYS_INLINE Buffer<T> sub(isize start, isize end) const
	{
		if (start < 0 || start >= len() || end >= len()) {
			tr::panicf(
				"index out of range: buffer.sub(%zu, %zu) when the length is "
				"only %zu",
				start, end, len()
			);
		}
	}

private:
	TR_ALWAYS_INLINE void _validate() const
	{
		// used by memfree to indicate use after free instead of an uninitialized buffer
		if (_len == as<usize>(-1)) [[unlikely]] {
			tr::panicf("tr::Buffer<T> has already been freed");
		}
		if (_ptr == nullptr) [[unlikely]] {
			tr::panicf("tr::Buffer<T> can't be null");
		}
	}

	T* _ptr = nullptr;
	usize _len = 0;

	template<typename T2>
	friend void memfree(Buffer<T2>& buf);
};

// deduction guidema
template<typename T, usize N>
Buffer(T (&)[N]) -> Buffer<T>;

// Allocates heap memory. Size is NOT in bytes
template<typename T>
inline Buffer<T> memnew_buffer(usize size)
{
	return Buffer<T>{memnew<T>(size), size, BufferInit::DEFAULT_INIT};
}

template<typename T>
inline void memfree(Buffer<T>& buf)
{
	// len of -1 == already freed
	if (buf.len() == as<usize>(-1)) {
		return;
	}

	T* ineedareferenceposthasteotherwisethecompilercries = buf.buf();
	tr::memfree(ineedareferenceposthasteotherwisethecompilercries);
	buf._ptr = nullptr;
	buf._len = as<usize>(-1);
}

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
constexpr void memreset(Buffer<T> dst)
{
	for (usize i = 0; i < dst.len(); i++) {
		dst[i] = T{};
	}
}

template<typename T1, typename T2>
constexpr bool memequal(Buffer<T1> a, Buffer<T2> b)
{
	if (a.len() != b.len()) {
		return false;
	}

	for (usize i = 0; i < a.len(); i++) {
		if (a[i] != b[i]) {
			return false;
		}
	}
	return true;
}

} // namespace tr

#endif
