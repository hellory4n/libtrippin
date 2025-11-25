/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/bits/scratch.cpp
 * ScratchArena is complicated enough to get its own file
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

#include "trippin/common.h"
#include "trippin/memory.h"
#include "trippin/string.h"

namespace tr {

namespace _tr {
	static thread_local byte scratch_buffer[SCRATCH_BACKING_BUFFER_SIZE];
	static thread_local byte* scratch_alloc_pos = scratch_buffer;
} // namespace _tr

} // namespace tr

tr::ScratchArena::ScratchArena()
{
#ifdef TR_HAS_ADDRESS_SANITIZER
	static bool hath_been_poisoned = false;
	if (!hath_been_poisoned) {
		TR_ASAN_POISON_MEMORY(_tr::scratch_buffer, SCRATCH_BACKING_BUFFER_SIZE);
		hath_been_poisoned = true;
	}
#endif
	_start_pos = _tr::scratch_alloc_pos;
}

void tr::ScratchArena::free()
{
	// FIXME this doesn't handle destructors but i can't be bothered

	// rollback
	TR_ASAN_UNPOISON_MEMORY(_start_pos, _allocated);
	tr::strlib::explicit_memset(_start_pos, _allocated, 0);
	TR_ASAN_POISON_MEMORY(_start_pos, _allocated);

	_fallback_arena.free();
	_allocated = 0;
}

void* tr::ScratchArena::alloc(usize size, usize align)
{
	usize padding = tr::ArenaPage::align_ptr(_tr::scratch_alloc_pos, align);
	usize available_space = static_cast<usize>(_tr::scratch_alloc_pos - _tr::scratch_buffer);
	if (available_space < padding + size) {
		// TODO this is extremely lazy, make the underlying buffer grow or some shit
		return _fallback_arena.alloc(size, align);
	}

	_tr::scratch_alloc_pos += padding + size;
	return _tr::scratch_alloc_pos;
}

usize tr::ScratchArena::allocated() const
{
	return _allocated;
}

usize tr::ScratchArena::capacity() const
{
	return SCRATCH_BACKING_BUFFER_SIZE;
}

void tr::ScratchArena::reset()
{
	this->free();
	*this = tr::ScratchArena{};
}
