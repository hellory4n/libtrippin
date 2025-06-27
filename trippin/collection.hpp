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
#include "string.hpp"

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

// Hashes an array of bytes, which is useful if you need to hash an array of bytes. Implemented with xxHash3
// 64-bits
uint64 hash(tr::Array<uint8> array);

// im losing my mind im going insane im watching my life go down the drain TODO better name its a miracle
// it works in the first place
template<typename K, typename V, uint64 (*HashFunc)(K key), usize InitialCapacity>
class AdvancedHashMap : public RefCounted
{
	static constexpr float64 LOAD_FACTOR = 0.5;

	struct Bucket
	{
		K key;
		V value;
		bool occupied;
		bool dead;
	};

	Bucket* buffer = nullptr;
	usize len = 0;
	usize cap = 0;

public:
	AdvancedHashMap() : cap(InitialCapacity)
	{
		this->buffer = reinterpret_cast<Bucket*>(calloc(InitialCapacity, sizeof(Bucket)));
		TR_ASSERT_MSG(this->buffer != nullptr, "couldn't allocate hashmap");
	}

	~AdvancedHashMap()
	{
		::free(this->buffer);
	}

	// Returns the index based on a key. That's how hash maps work.
	usize get_index(K key)
	{
		return HashFunc(key) % this->cap;
	}

	// Checks how full the hashmap is and resizes if necessary
	void grow()
	{
		float64 used = static_cast<float64>(this->len) / this->cap;
		if (used <= LOAD_FACTOR) {
			return;
		}

		usize old_cap = this->cap;
		this->cap *= 2;
		Bucket* new_buffer = reinterpret_cast<Bucket*>(calloc(this->cap, sizeof(Bucket)));
		TR_ASSERT_MSG(this->buffer != nullptr, "couldn't grow hashmap");

		// changing the capacity fucks with the hashing
		// so we have to copy everything
		for (usize i = 0; i < old_cap; i++) {
			Bucket* bucket = &this->buffer[i];
			if (!bucket->occupied) {
				continue;
			}

			usize idx = this->get_index(bucket->key);
			for (usize i = idx; i < this->cap; i++) {
				Bucket* new_bucket = &new_buffer[i];

				if (!new_bucket->occupied) {
					new_bucket->occupied = true;
					new_bucket->dead = bucket->dead;
					new_bucket->key = bucket->key;
					new_bucket->value = bucket->value;
				}
			}
		}

		::free(this->buffer);
		this->buffer = new_buffer;
	}

	V& operator[](K key)
	{
		// operator[] is also used for putting crap :)
		this->grow();

		usize idx = this->get_index(key);

		for (usize i = idx; i < this->cap; i++) {
			Bucket* bucket = &this->buffer[i];

			// it immediately dies for null strings which are like most of them
			if (bucket->occupied) {
				if (bucket->key == key) {
					return bucket->value;
				}
			}

			if (!bucket->occupied) {
				bucket->occupied = true;
				bucket->key = key;
				this->len++;
				return bucket->value;
			}
		}

		tr::panic("what the fuck"); // should never happen probably maybe hopefully probably
	}

	// If true, the hashmap has that key. Useful because the `[]` operator automatically inserts an item if
	// it's not there.
	bool contains(K key)
	{
		usize idx = this->get_index(key);

		for (usize i = idx; i < this->cap; i++) {
			Bucket* bucket = &this->buffer[i];

			if (bucket->key == key && bucket->occupied) {
				return true;
			}

			if (!bucket->occupied) {
				return false;
			}
		}

		return false;
	}

	// Removes the key from the hashmap. Returns true if the key is was found, returns false otherwise.
	bool remove(K key)
	{
		usize idx = this->get_index(key);

		for (usize i = idx; i < this->cap; i++) {
			Bucket* bucket = &this->buffer[i];

			if (bucket->key == key && bucket->occupied) {
				bucket->dead = true;
				return true;
			}

			if (!bucket->occupied) {
				return false;
			}
		}

		return false;
	}

	// fucking iterator
	class Iterator {
	public:
		Iterator(Bucket* ptr, usize index) : idx(index), ptr(ptr) {}
		Pair<K, V> operator*() const { return {this->ptr[idx].key, this->ptr[idx].value}; }
		Iterator& operator++() {
			while (!this->ptr[idx].occupied) {
				this->idx++;
				this->ptr++;
			}
			return *this;
		}
		bool operator!=(const Iterator& other) const { return ptr != other.ptr; }
	private:
		usize idx;
		Bucket* ptr;
	};

	Iterator begin() const { return Iterator(this->buffer, 0); }
	Iterator end()   const { return Iterator(this->buffer + this->cap, this->cap); }
};

// internal don't use probably :)
template<typename T> uint64 __default_hash_function(T key)
{
	return tr::hash(Array<uint8>(reinterpret_cast<uint8*>(&key), sizeof(T)));
}
// internal don't use probably :)
template<> inline uint64 __default_hash_function<String>(String key)
{
	return tr::hash(Array<uint8>(reinterpret_cast<uint8*>(key.buffer()), key.length()));
}

// ahahsmhap :DD if you're interested this works with open addressing and linear probing, i'll probably
// change it if my brain expands to megamind levels of brain
template<typename K, typename V>
using HashMap = AdvancedHashMap<K, V, __default_hash_function, 256>;

}

#endif