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

#include <cstddef>

#include "trippin/platform.h"
#include "trippin/typedef.h"

namespace tr {

/// Allocates memory on the heap, and returns null on failure. You should usually use the higher
/// level allocators such as `tr::HeapAlloc`
void* memnew(usize size, usize align = alignof(max_align_t));

/// `tr::memnew<char>(123)` looks better than `(char*)tr::memnew(123)`, I think
template<typename T>
TR_ALWAYS_INLINE T* memnew(usize size)
{
	return (T*)memnew(size, alignof(T));
}

// the reference fucks with the type system so we have to use a template :(
void _impl_memfree(void*& ptr);

/// Deletes a pointer and sets it to null.
template<typename T>
TR_ALWAYS_INLINE void memfree(T* ptr)
{
	_impl_memfree((void*&)ptr);
}

} // namespace tr

#endif
