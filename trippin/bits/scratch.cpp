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

// implementation for the scratch arena's backing buffer except not really bcuz half of it is
// implemented in ScratchArena
class _scratchBuffer : public Arena
{
	friend class ScratchArena;

public:
	_scratchBuffer()
	{
		void* page_ptr = malloc(sizeof(ArenaPage));
		TR_ASSERT_MSG(page_ptr != nullptr, "minor inconvenience");
		_page = new (page_ptr) ArenaPage(ArenaSettings{}, SCRATCH_BACKING_BUFFER_SIZE);
	}

	// so it's automatically freed as a static var
	~_scratchBuffer()
	{
		this->free();
	}

	void* alloc(usize size, usize align = alignof(max_align_t)) TR_LIFETIMEBOUND override;
	usize allocated() const override;
	usize capacity() const override;

	[[noreturn]]
	void reset() override
	{
		tr::panic("dumbass you can't do that on a tr::_scratchBuffer");
	}
};

thread_local _scratchBuffer _scratch_buffer{};

} // namespace tr

tr::ScratchArena::ScratchArena()
{
	_start_alloc_pos = tr::_scratch_buffer._page->alloc_pos +
			   reinterpret_cast<byte*>(tr::_scratch_buffer._page);
	_end_alloc_pos = _start_alloc_pos;
	_start_page = tr::_scratch_buffer._page;
}

void tr::ScratchArena::free()
{
	// did everything happen on the same page?
	if (_start_page == tr::_scratch_buffer._page) {
		tr::strlib::explicit_memset(
			reinterpret_cast<void*>(_start_alloc_pos),
			static_cast<usize>(_end_alloc_pos - _start_alloc_pos), 0
		);
		// scratcharena's alloc pos includes the page's address, the page doesn't do that
		_start_page->alloc_pos = static_cast<usize>(
			_start_alloc_pos - reinterpret_cast<byte*>(tr::_scratch_buffer._page)
		);
	}
	else {
		ArenaPage* head = tr::_scratch_buffer._page;
		// tr::_scratchBuffer can't have a null page, it always has smth allocated
		while (head->prev != _start_page) {
			// TODO reuse the pages
			// for now i dont wanna deal with making _scratchBuffer::alloc() or
			// Arena::alloc support that, so just delete them
			ArenaPage* old_head = head;
			head = head->prev;

			tr::_scratch_buffer._capacity -= old_head->bufsize;
			old_head->free();
			std::free(old_head);
		}

		tr::strlib::explicit_memset(
			_start_alloc_pos,
			_start_page->bufsize - reinterpret_cast<usize>(_start_alloc_pos) -
				reinterpret_cast<usize>(_start_page),
			0
		);
		_start_page->alloc_pos = reinterpret_cast<usize>(_start_alloc_pos) -
					 reinterpret_cast<usize>(tr::_scratch_buffer._page);
	}
}

void* tr::ScratchArena::alloc(usize size, usize align)
{
	void* ptr = tr::_scratch_buffer.alloc(size, align);
	_end_alloc_pos = tr::_scratch_buffer._page->alloc_pos +
			 reinterpret_cast<byte*>(tr::_scratch_buffer._page);

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
