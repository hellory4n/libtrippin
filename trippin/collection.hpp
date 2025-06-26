/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/collection.hpp
 * Additional collection types
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

#ifndef _TRIPPIN_COLLECTION_H
#define _TRIPPIN_COLLECTION_H

#include <stdlib.h>

#include "memory.hpp"
#include "log.hpp"

namespace tr {

// Dynamically sized list. You can add and remove items to it, something you can't do to a regular
// tr::Array<T>
template<typename T>
class List : public RefCounted
{
	usize len;
	usize cap;
	T* ptr = nullptr;

	static constexpr usize INITIAL_CAPACITY = 8;

public:
	List() : len(0), cap(INITIAL_CAPACITY)
	{
		this->ptr = reinterpret_cast<T*>(calloc(INITIAL_CAPACITY, sizeof(T)));
		TR_ASSERT_MSG(this->ptr != nullptr, "couldn't allocate list");
	}

	~List()
	{
		::free(this->ptr);
		this->ptr = nullptr;
	}

	// Returns the length of the list
	usize length() const { return this->len; }

	// Returns how many items the list can fit without resizing
	usize capacity() const { return this->cap; }

	// Adds an item to the end of the list
	void add(T val)
	{
		// does it already fit?
		if (this->len < this->cap) {
			(*this)[this->len++] = val;
			return;
		}

		// resize :)
		this->cap *= 2;
		this->ptr = reinterpret_cast<T*>(realloc(this->ptr, sizeof(T) * this->cap));
		(*this)[this->len++] = val;
	}

	// Reserves space in the list
	void reserve(usize items)
	{
		// TODO could be better
		this->cap += items * 2;
		this->ptr = realloc(this->ptr, sizeof(T) * this->cap);
	}

	// Returns the last item in the list
	T& last() const { return (*this)[this->len - 1]; }

	T* buffer() const { return this->ptr; }

	T& operator[](usize idx) const
	{
		if (idx >= this->len) {
			tr::panic("index out of range: %zu in a list of %zu", idx, this->len);
		}
		return this->ptr[idx];
	}

	// fucking iterator
	class Iterator {
	public:
		Iterator(T* ptr, usize index) : idx(index), ptr(ptr) {}
		ArrayItem<T> operator*() const { return {this->idx, *this->ptr}; }
		Iterator& operator++() { this->ptr++; this->idx++; return *this; }
		bool operator!=(const Iterator& other) const { return ptr != other.ptr; }
	private:
		usize idx;
		T* ptr;
	};

	Iterator begin() const { return Iterator(this->buffer(), 0); }
	Iterator end()   const { return Iterator(this->buffer() + this->length(), this->length()); }
};

}

#endif