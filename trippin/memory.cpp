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

// clangd are you stupid
#include <new> // IWYU pragma: keep
#include <stdlib.h>

#include "log.hpp"
#include "math.hpp"

#include "memory.hpp"

namespace tr {
	extern MemoryInfo memory_info;
	thread_local Arena __the_real_scratchpad(tr::kb_to_bytes(4));
}

tr::ArenaPage::ArenaPage(usize size, usize align) : bufsize(size), alignment(align)
{
	TR_ASSERT(size != 0);

	// man.
	this->buffer = ::operator new(size, std::align_val_t(align), std::nothrow);
	this->alloc_pos = 0;
	this->next = nullptr;
	this->prev = nullptr;

	// i don't think you can recover from that
	// so just die
	TR_ASSERT_MSG(this->buffer != nullptr, "couldn't allocate arena page of %zu B (%zu KB, %zu MB)",
		size, tr::bytes_to_kb(size), tr::bytes_to_mb(size)
	);

	// i dont want to read garbage man
	memset(this->buffer, 0, this->bufsize);

	// man
	tr::memory_info.alive_pages++;
	tr::memory_info.cumulative_pages++;
	tr::memory_info.cumulative_allocated += size;
	tr::memory_info.allocated += size;
}

tr::ArenaPage::~ArenaPage()
{
	if (this->buffer != nullptr) {
		::operator delete(this->buffer, std::align_val_t(this->alignment), std::nothrow);
		this->buffer = nullptr;

		// man
		tr::memory_info.alive_pages--;
		tr::memory_info.freed_pages++;
		tr::memory_info.allocated -= this->bufsize;
		tr::memory_info.freed_by_arenas += this->bufsize;
	}
}

usize tr::ArenaPage::available_space() const
{
	return this->bufsize - this->alloc_pos;
}

void* tr::ArenaPage::alloc(usize size, usize align)
{
	uint8* base = reinterpret_cast<uint8*>(this->buffer);
	uint8* ptr = base + this->alloc_pos;
	usize address = reinterpret_cast<usize>(ptr);

	// fucking padding aligning fuckery
	usize misalignment = address & (align - 1);
	usize padding = misalignment ? (align - misalignment) : 0;

	// consider not segfaulting
	if (this->available_space() < padding + size) {
		return nullptr;
	}

	// ma
	this->alloc_pos += padding;
	void* aligned_ptr = base + this->alloc_pos;
	this->alloc_pos += size;
	return aligned_ptr;
}

tr::Arena::Arena(usize pg_size) : page_size(pg_size)
{
	// it doesn't make a page until you allocate something
	TR_ASSERT_MSG(this->page_size != 0, "you doofus why would you make an arena of 0 bytes");
}

tr::Arena::~Arena()
{
	// it doesn't make a page until you allocate something
	if (this->page == nullptr) return;

	// :)
	this->call_destructors();

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

void* tr::Arena::alloc(usize size, usize align)
{
	// does it fit in the current page?
	if (this->page != nullptr) {
		void* ptr = this->page->alloc(size, align);
		if (ptr != nullptr) {
			this->bytes_allocated += size;
			return ptr;
		}
	}

	// it doesn't fit, make a new page
	usize new_pg_size = tr::max(this->page_size, size + align);
	ArenaPage* new_page = new (std::nothrow) ArenaPage(new_pg_size, align);
	TR_ASSERT_MSG(new_page != nullptr, "couldn't create new arena page");

	// dude... dude... dude... what? GOOD NEWS OGOD NEANEWS OGOOD NEWS NFOGOSJRJIGIRISJTOAEOTGOAGOKAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
	new_page->prev = this->page;
	if (this->page != nullptr) this->page->next = new_page;
	this->page = new_page;
	this->bytes_capacity += new_pg_size;

	// actually allocate frfrfrfr no cap ong icl
	void* ptr = this->page->alloc(size, align);
	TR_ASSERT_MSG(ptr != nullptr, "couldn't allocate %zu B in arena (%zu KB, %zu MB)",
		size, tr::bytes_to_kb(size), tr::bytes_to_mb(size)
	);
	this->bytes_allocated += size;
	return ptr;
}

void tr::Arena::call_destructors()
{
	// yea
	while (this->destructors != nullptr) {
		// idfk why it does that
		if (this->destructors->object == nullptr) break;

		this->destructors->func(this->destructors->object);
		this->destructors = this->destructors->next;
	}
}

void tr::Arena::reset()
{
	// it doesn't make a page until you allocate something
	if (this->page == nullptr) return;

	ArenaPage* head = this->page;
	while (head->prev != nullptr) {
		head = head->prev;
	}

	// TODO we should reuse all the other pages
	// i just can't be bothered to fix Arena.alloc() to support that
	ArenaPage* headfrfr = head;
	while (head != nullptr && head != headfrfr) {
		this->bytes_capacity -= head->bufsize;
		ArenaPage* next = head->next;
		delete head;
		head = next;
	}

	// we keep the first page :)
	this->page = headfrfr;
	// apparently memset is fucked, std::fill_n does nothing and memset_s just doesn't exist in c++??
	// source https://en.cppreference.com/w/cpp/string/byte/memset#Notes
	// maybe i'm just stupid :)
	for (usize i = 0; i < headfrfr->bufsize; i++) {
		reinterpret_cast<uint8*>(headfrfr->buffer)[i] = 0;
	}
	headfrfr->alloc_pos = 0;
	headfrfr->prev = nullptr;
	headfrfr->next = nullptr;
	this->bytes_allocated = 0;
}

usize tr::Arena::allocated() const
{
	return this->bytes_allocated;
}

usize tr::Arena::capacity() const
{
	return this->bytes_capacity;
}

[[deprecated("this api is just kinda crap")]]
tr::MemoryInfo tr::get_memory_info()
{
	return tr::memory_info;
}

tr::Arena& tr::scratchpad()
{
	return tr::__the_real_scratchpad;
}
