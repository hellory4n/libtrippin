/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/memory.cpp
 * Reference counting, arenas, arrays, and a few utilities
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

#include <stdlib.h>

#include "log.hpp"
#include "math.hpp"

#include "memory.hpp"

namespace tr {
	extern MemoryInfo memory_info;
}

void tr::RefCounted::retain() const
{
	// man
	// probably was just birthed into this world
	if (this->count == 0) {
		tr::memory_info.cumulative_ref_counted_objs++;
		tr::memory_info.ref_counted_objs++;
	}
	this->count++;
}

void tr::RefCounted::release() const
{
	if (--this->count == 0) {
		delete this;
		tr::memory_info.freed_ref_counted_objs++;
		tr::memory_info.ref_counted_objs--;
	}
}

tr::ArenaPage::ArenaPage(usize size)
{
	TR_ASSERT(size != 0);

	this->buffer = calloc(1, size);
	this->size = size;
	this->alloc_pos = 0;
	this->next = nullptr;
	this->prev = nullptr;

	// i don't think you can recover from that
	// so just die
	TR_ASSERT_MSG(this->buffer != nullptr, "couldn't allocate arena page");

	// man
	tr::memory_info.alive_pages++;
	tr::memory_info.cumulative_pages++;
	tr::memory_info.cumulative_allocated += size;
	tr::memory_info.allocated += size;
}

tr::ArenaPage::~ArenaPage()
{
	if (this->buffer != nullptr) {
		// tr:: also has a function called free
		::free(this->buffer);
		this->buffer = nullptr;

		// man
		tr::memory_info.alive_pages--;
		tr::memory_info.freed_pages++;
		tr::memory_info.allocated -= this->size;
		tr::memory_info.freed_by_arenas += this->size;
	}
}

usize tr::ArenaPage::available_space()
{
	return this->size - this->alloc_pos;
}

tr::Arena::Arena(usize page_size)
{
	this->page_size = page_size;
	this->page = new ArenaPage(page_size);
}

tr::Arena::~Arena()
{
	// likely uninitialized, likely intentional
	// if it's not intentional then it'd already crashed with the other functions
	if (this->page_size == 0) return;

	ArenaPage* head = this->page;
	while (head->prev != nullptr) {
		head = head->prev;
	}

	while (head != nullptr) {
		ArenaPage* next = head->next;
		delete head;
		head = next;
	}
}

void* tr::Arena::alloc(usize size)
{
	TR_ASSERT_MSG(this->page_size != 0, "you doofus this arena is very likely uninitialized");

	// does it fit in the current page?
	if (this->page->available_space() >= size) {
		void* val = reinterpret_cast<uint8*>(this->page->buffer) + this->page->alloc_pos;
		this->page->alloc_pos += size;
		return val;
	}

	// does it fit in the previous page?
	if (this->page->prev != nullptr) {
		ArenaPage* prev_page = this->page->prev;
		if (prev_page->available_space() >= size) {
			void* val = reinterpret_cast<uint8*>(prev_page->buffer) + prev_page->alloc_pos;
			prev_page->alloc_pos += size;
			return val;
		}
	}

	// does it fit in a regularly sized page?
	if (this->page_size >= size) {
		ArenaPage* new_page = new ArenaPage(this->page_size);
		new_page->prev = this->page;
		this->page->next = new_page;
		this->page = new_page;

		void* val = reinterpret_cast<uint8*>(new_page->buffer) + new_page->alloc_pos;
		new_page->alloc_pos += size;
		return val;
	}

	// last resort is making a new page with that size
	ArenaPage* new_page = new ArenaPage(size);
	new_page->prev = this->page;
	this->page->next = new_page;
	this->page = new_page;

	void* val = reinterpret_cast<uint8*>(new_page->buffer) + new_page->alloc_pos;
	new_page->alloc_pos += size;
	return val;
}

void tr::Arena::prealloc(usize size)
{
	TR_ASSERT_MSG(this->page_size != 0, "you doofus this arena is very likely uninitialized");

	// does it already fit?
	if (this->page->available_space() >= size) {
		return;
	}

	if (this->page->prev != nullptr) {
		ArenaPage* prev_page = this->page->prev;
		if (prev_page->available_space() >= size) {
			return;
		}
	}

	// make a new page without increasing alloc_pos
	ArenaPage* new_page = new ArenaPage(tr::max(size, this->page_size));
	new_page->prev = this->page;
	this->page->next = new_page;
	this->page = new_page;
}

tr::MemoryInfo tr::get_memory_info()
{
	return tr::memory_info;
}
