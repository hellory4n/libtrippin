/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/memory.cpp
 * Arenas, arrays, and a few utilities
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

#include "trippin/memory.h"

#include <cstdlib>

#include "trippin/bits/scratch.cpp" // yea
#include "trippin/common.h"
#include "trippin/log.h"
#include "trippin/math.h"
#include "trippin/string.h"

namespace tr {

thread_local Arena _the_real_scratchpad({.page_size = tr::kb_to_bytes(4)});

}

tr::ArenaPage::ArenaPage(tr::ArenaSettings settings, usize size)
	: bufsize(size)
{
	TR_ASSERT(size != 0);

	if (settings.zero_initialize) {
		this->buffer = std::calloc(1, size);
	}
	else {
		this->buffer = std::malloc(size);
	}
	this->alloc_pos = 0;
	this->next = nullptr;
	this->prev = nullptr;

	if (buffer == nullptr) {
		if (settings.error_behavior == ArenaSettings::ErrorBehavior::PANIC) {
			tr::panic(
				"couldn't allocate arena page of %zu B (%zu KB, %zu MB)", size,
				tr::bytes_to_kb(size), tr::bytes_to_mb(size)
			);
		}
	}

	// should be inaccessible before any .alloc() call
	TR_ASAN_POISON_MEMORY(this->buffer, size);
}

void tr::ArenaPage::free()
{
	if (this->buffer != nullptr) {
		std::free(buffer);
		this->buffer = nullptr;
	}
}

usize tr::ArenaPage::available_space() const
{
	return this->bufsize - this->alloc_pos;
}

usize tr::ArenaPage::align_ptr(const void* base, usize align)
{
	usize address = reinterpret_cast<usize>(base);
	usize misalignment = address & (align - 1);
	return misalignment != 0 ? (align - misalignment) : 0;
}

void* tr::ArenaPage::alloc(usize size, usize align)
{
	byte* base = static_cast<byte*>(buffer);
	byte* ptr = base + this->alloc_pos;
	usize padding = tr::ArenaPage::align_ptr(ptr, align);

	// consider not segfaulting
	if (available_space() < padding + size) {
		return nullptr;
	}

	// ma
	alloc_pos += padding;
	void* aligned_ptr = base + alloc_pos;
	// it's accessible now
	TR_ASAN_UNPOISON_MEMORY(aligned_ptr, size);
	alloc_pos += size;
	return aligned_ptr;
}

tr::Arena::Arena(ArenaSettings settings)
	: _settings(settings)
{
	// it doesn't make a page until you allocate something
	if (_settings.error_behavior == ArenaSettings::ErrorBehavior::PANIC) {
		if (_settings.page_size == 0) [[unlikely]] {
			tr::panic("you doofus why would you make an arena of 0 bytes");
		}
		if (_settings.max_pages.is_valid()) {
			if (settings.max_pages.unwrap() == 0) [[unlikely]] {
				tr::panic("max arena pages is set to 0, can't allocate anything");
			}
		}
	}
}

void tr::Arena::free()
{
	// it doesn't make a page until you allocate something
	if (_page == nullptr) {
		return;
	}

	// :)
	_call_destructors();

	ArenaPage* head = _page;
	while (head->prev != nullptr) {
		head = head->prev;
	}

	while (head != nullptr) {
		ArenaPage* next = head->next;
		head->free();
		std::free(head);
		head = next;
	}
}

void* tr::Arena::alloc(usize size, usize align)
{
	// does it fit in the current page?
	if (_page != nullptr) {
		void* ptr = _page->alloc(size, align);
		if (ptr != nullptr) {
			_allocated += size;
			return ptr;
		}
	}

	// can we make a new page?
	if (_settings.max_pages.is_valid()) {
		usize max_pages = _settings.max_pages.unwrap();
		if (_pages == max_pages) {
			if (_settings.error_behavior == ArenaSettings::ErrorBehavior::PANIC) {
				tr::panic(
					"arena out of pages! (%zu pages * %zu size = %zu "
					"available)",
					_pages, _settings.page_size, _pages * _settings.page_size
				);
			}
			else {
				return nullptr;
			}
		}
	}

	// it doesn't fit, make a new page
	usize new_page_size = tr::max(_settings.page_size, size + align);
	void* new_page_ptr = std::malloc(sizeof(ArenaPage));
	TR_ASSERT_MSG(new_page_ptr != nullptr, "couldn't create new arena page");
	ArenaPage* new_page = new (new_page_ptr) ArenaPage(_settings, new_page_size);

	new_page->prev = _page;
	if (_page != nullptr) {
		_page->next = new_page;
	}
	_page = new_page;
	_capacity += new_page_size;
	_pages++;

	// actually allocate frfrfrfr no cap ong icl
	void* ptr = _page->alloc(size, align);
	if (ptr == nullptr) {
		if (_settings.error_behavior == ArenaSettings::ErrorBehavior::PANIC) {
			tr::panic(
				"couldn't allocate %zu B in arena (%zu KB, %zu MB)", size,
				tr ::bytes_to_kb(size), tr ::bytes_to_mb(size)
			);
		}
		else {
			return nullptr;
		}
	};
	_allocated += size;
	return ptr;
}

void tr::Arena::_call_destructors()
{
	// yea
	while (_destructors != nullptr) {
		// idfk why it does that
		if (_destructors->object == nullptr) {
			break;
		}

		_destructors->func(_destructors->object);
		_destructors = _destructors->next;
	}
}

void tr::Arena::reset()
{
	// it doesn't make a page until you allocate something
	if (_page == nullptr) {
		return;
	}

	_call_destructors();

	ArenaPage* head = _page;
	while (head->prev != nullptr) {
		head = head->prev;
	}

	// TODO we should reuse all the other pages
	// i just can't be bothered to fix Arena::alloc() to support that
	ArenaPage* headfrfr = head;
	while (head != nullptr && head != headfrfr) {
		_capacity -= head->bufsize;
		ArenaPage* next = head->next;
		head->free();
		std::free(head);
		head = next;
	}

	// we keep the first page :)
	_page = headfrfr;
	if (_settings.zero_initialize) {
		TR_ASAN_UNPOISON_MEMORY(headfrfr->buffer, headfrfr->bufsize);
		tr::strlib::explicit_memset(headfrfr->buffer, headfrfr->bufsize, 0);
		TR_ASAN_POISON_MEMORY(headfrfr->buffer, headfrfr->bufsize);
	}
	headfrfr->alloc_pos = 0;
	headfrfr->prev = nullptr;
	headfrfr->next = nullptr;
	_allocated = 0;
}

usize tr::Arena::allocated() const
{
	return this->_allocated;
}

usize tr::Arena::capacity() const
{
	return this->_capacity;
}

tr::WrapArena::WrapArena(usize size)
	: Arena(ArenaSettings{
		  .page_size = size,
		  .max_pages = 1,
		  .zero_initialize = true,
		  .error_behavior = ArenaSettings::ErrorBehavior::PANIC,
	  })
{
	// force the underlying arena to make a page already
	(void)Arena::alloc(1);
}

void tr::WrapArena::free()
{
	Arena::free();
}

void* tr::WrapArena::alloc(usize size, usize align)
{
	TR_ASSERT(_page);

	void* ptr = _page->alloc(size, align);
	if (ptr != nullptr) {
		return ptr;
	}

	// it busted
	if (capacity() < size) {
		tr::panic("can't allocate %zu B in wrap arena of %zu B", size, capacity());
	}

	// TODO slowly poison the memory which is about to be overwritten
	// for now i can't be bothered
	_page->alloc_pos = 0;
	return _page->alloc(size, align);
}

tr::Arena& tr::scratchpad()
{
	return tr::_the_real_scratchpad;
}
