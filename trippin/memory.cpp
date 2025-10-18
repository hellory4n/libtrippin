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

#include "trippin/memory.h"

#include <new>

#include "common.h"
#include "trippin/log.h"
#include "trippin/math.h"

namespace tr {

thread_local Arena _the_real_scratchpad({.page_size = tr::kb_to_bytes(4)});

}

tr::ArenaPage::ArenaPage(tr::ArenaSettings settings, usize size, usize align)
	: bufsize(size)
	, alignment(align)
{
	TR_ASSERT(size != 0);

	// man.
	this->buffer = ::operator new(size, std::align_val_t(align), std::nothrow);
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

	if (settings.zero_initialize) {
		memset(this->buffer, 0, this->bufsize);
	}
}

void tr::ArenaPage::free()
{
	if (this->buffer != nullptr) {
		::operator delete(this->buffer, std::align_val_t(this->alignment), std::nothrow);
		this->buffer = nullptr;
	}
}

usize tr::ArenaPage::available_space() const
{
	return this->bufsize - this->alloc_pos;
}

void* tr::ArenaPage::alloc(usize size, usize align)
{
	uint8* base = static_cast<uint8*>(this->buffer);
	uint8* ptr = base + this->alloc_pos;
	usize address = reinterpret_cast<usize>(ptr);

	// fucking padding aligning fuckery
	usize misalignment = address & (align - 1);
	usize padding = misalignment != 0 ? (align - misalignment) : 0;

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

tr::Arena::Arena(ArenaSettings settings)
	: _settings(settings)
{
	// it doesn't make a page until you allocate something
	if (_settings.error_behavior == ArenaSettings::ErrorBehavior::PANIC) {
		TR_ASSERT_MSG(
			_settings.page_size != 0,
			"you doofus why would you make an arena of 0 bytes"
		);
	}
	else {
		tr::warn("using arena with a page size of 0 bytes, will likely fail");
	}
}

void tr::Arena::free()
{
	// it doesn't make a page until you allocate something
	if (this->_page == nullptr) {
		return;
	}

	// :)
	this->_call_destructors();

	ArenaPage* head = this->_page;
	while (head->prev != nullptr) {
		head = head->prev;
	}

	while (head != nullptr) {
		ArenaPage* next = head->next;
		delete head;
		head = next;
	}
}

bool tr::Arena::_initialized()
{
	return _page != nullptr;
}

void* tr::Arena::alloc(usize size, usize align)
{
	// does it fit in the current page?
	if (this->_page != nullptr) {
		void* ptr = this->_page->alloc(size, align);
		if (ptr != nullptr) {
			this->_allocated += size;
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
	ArenaPage* new_page = new (std::nothrow) ArenaPage(_settings, new_page_size, align);
	TR_ASSERT_MSG(new_page != nullptr, "couldn't create new arena page");

	new_page->prev = _page;
	if (_page != nullptr) {
		_page->next = new_page;
	}
	_page = new_page;
	_capacity += new_page_size;
	_pages++;

	// actually allocate frfrfrfr no cap ong icl
	void* ptr = this->_page->alloc(size, align);
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
	this->_allocated += size;
	return ptr;
}

void tr::Arena::_call_destructors()
{
	// yea
	while (this->_destructors != nullptr) {
		// idfk why it does that
		if (this->_destructors->object == nullptr) {
			break;
		}

		this->_destructors->func(this->_destructors->object);
		this->_destructors = this->_destructors->next;
	}
}

void tr::Arena::reset()
{
	// it doesn't make a page until you allocate something
	if (this->_page == nullptr) {
		return;
	}

	ArenaPage* head = this->_page;
	while (head->prev != nullptr) {
		head = head->prev;
	}

	// TODO we should reuse all the other pages
	// i just can't be bothered to fix Arena::alloc() to support that
	ArenaPage* headfrfr = head;
	while (head != nullptr && head != headfrfr) {
		this->_capacity -= head->bufsize;
		ArenaPage* next = head->next;
		delete head;
		head = next;
	}

	// we keep the first page :)
	this->_page = headfrfr;
	// TODO see https://en.cppreference.com/w/cpp/string/byte/memset#Notes
	if (_settings.zero_initialize) {
		memset(headfrfr->buffer, 0, headfrfr->bufsize);
	}
	headfrfr->alloc_pos = 0;
	headfrfr->prev = nullptr;
	headfrfr->next = nullptr;
	this->_allocated = 0;
}

usize tr::Arena::allocated() const
{
	return this->_allocated;
}

usize tr::Arena::capacity() const
{
	return this->_capacity;
}

tr::Arena& tr::scratchpad()
{
	return tr::_the_real_scratchpad;
}
