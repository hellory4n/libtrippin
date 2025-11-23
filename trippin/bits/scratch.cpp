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

#include <cstdlib>

#include "trippin/common.h"
#include "trippin/memory.h"
#include "trippin/string.h"

namespace tr {

// implementation for the scratch arena's backing buffer except not really bcuz half (most) of it is
// implemented in ScratchArena
class _scratchBuffer : public Arena
{
	friend class ScratchArena;

public:
	_scratchBuffer()
		: Arena(ArenaSettings{
			  .page_size = SCRATCH_BACKING_BUFFER_SIZE,
			  .max_pages = {},
			  .zero_initialize = true,
			  .error_behavior = ArenaSettings::ErrorBehavior::PANIC,
		  })
	{
		// ScratchArena expects *some* page to exist, force the base arena to make one
		(void)alloc(1);
	}

	// so it's automatically freed as a static var
	~_scratchBuffer()
	{
		this->free();
	}
};

thread_local _scratchBuffer _scratch_buffer{};

} // namespace tr

tr::ScratchArena::ScratchArena()
{
	_start_page = _scratch_buffer._page;
	_start_offset = _start_page->alloc_pos;
	_allocated = 0;
}

void tr::ScratchArena::free()
{
	// FIXME this doesn't handle destructors but i can't be bothered

	ArenaPage* current = _scratch_buffer._page;

	// if everything happened on the same page, just go back
	if (current == _start_page) {
		byte* base = static_cast<byte*>(_start_page->buffer);
		byte* start_ptr = base + _start_offset;
		byte* end_ptr = base + _start_page->alloc_pos;

		TR_ASAN_UNPOISON_MEMORY(start_ptr, static_cast<usize>(end_ptr - start_ptr));
		tr::strlib::explicit_memset(start_ptr, static_cast<usize>(end_ptr - start_ptr), 0);
		TR_ASAN_POISON_MEMORY(start_ptr, static_cast<usize>(end_ptr - start_ptr));

		_start_page->alloc_pos = _start_offset;
	}
	// if it used more pages then free those
	else {
		while (current != _start_page) {
			ArenaPage* prev = current->prev;

			_scratch_buffer._pages--;
			_scratch_buffer._capacity -= current->bufsize;
			current->free();
			std::free(current);

			current = prev;
		}

		// then go back at the starting page
		byte* base = static_cast<byte*>(_start_page->buffer);
		byte* start_ptr = base + _start_offset;
		byte* end_ptr = base + _start_page->alloc_pos;

		TR_ASAN_UNPOISON_MEMORY(start_ptr, static_cast<usize>(end_ptr - start_ptr));
		tr::strlib::explicit_memset(start_ptr, static_cast<usize>(end_ptr - start_ptr), 0);
		TR_ASAN_POISON_MEMORY(start_ptr, static_cast<usize>(end_ptr - start_ptr));

		_start_page->alloc_pos = _start_offset;
		_start_page->next = nullptr;
		_scratch_buffer._page = _start_page;
	}

	_scratch_buffer._allocated -= _allocated;
	_allocated = 0;
}

void* tr::ScratchArena::alloc(usize size, usize align)
{
	void* ptr = tr::_scratch_buffer.alloc(size, align);

	// ArenaPage::alloc except without the alloc part
	usize address = reinterpret_cast<usize>(ptr);
	usize misalignment = address & (align - 1);
	usize padding = misalignment != 0 ? (align - misalignment) : 0;
	_allocated += padding + size;

	return ptr;
}

usize tr::ScratchArena::allocated() const
{
	return _allocated;
}

usize tr::ScratchArena::capacity() const
{
	return tr::_scratch_buffer.capacity();
}

void tr::ScratchArena::reset()
{
	this->free();
	*this = tr::ScratchArena{};
}
