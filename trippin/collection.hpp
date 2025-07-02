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
	usize len = 0;
	usize cap = 0;
	// void* bcuz gcc says "warning: 'void* realloc(void*, size_t)' moving an object of non-trivially copyable
	// type 'class tr::String'; use 'new' and 'delete' instead" shut the fuck up man
	void* ptr = nullptr;

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

	T* buffer() const { return reinterpret_cast<T*>(this->ptr); }

	T& operator[](usize idx) const
	{
		if (idx >= this->len) {
			tr::panic("index out of range: %zu in a list of %zu", idx, this->len);
		}
		return reinterpret_cast<T*>(this->ptr)[idx];
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

// TODO StringBuilder?

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

	void grow()
	{
		usize old_cap = this->cap;
		this->cap *= 2;
		Bucket* new_buffer = reinterpret_cast<Bucket*>(calloc(this->cap, sizeof(Bucket)));
		TR_ASSERT_MSG(new_buffer != nullptr, "couldn't grow hashmap");

		// changing the capacity fucks with the hashing
		// so we have to copy everything to new indexes
		for (usize i = 0; i < old_cap; i++) {
			Bucket& old_bucket = buffer[i];
			if (!old_bucket.occupied) continue;

			usize idx = this->get_index(old_bucket.key);

			// linear probe with wrap‑around
			for (usize probe = idx; ; probe = (probe + 1) % this->cap) {
				Bucket& new_bucket = new_buffer[probe];
				if (!new_bucket.occupied) {
					new_bucket.occupied = true;
					new_bucket.dead = old_bucket.dead;
					new_bucket.key = old_bucket.key;
					new_bucket.value = old_bucket.value;
					break;
				}
			}
		}

		::free(this->buffer);
		this->buffer = new_buffer;
	}

	// Checks how full the hashmap is and resizes if necessary
	void check_grow()
	{
		float64 used = static_cast<float64>(this->len) / this->cap;
		if (used <= LOAD_FACTOR) {
			return;
		}
		this->grow();
	}

	// Returns the bucket and whether it's occupied (true for occupied, false for empty)
	Pair<Bucket*, bool> find(K key)
	{
		usize idx = this->get_index(key);
		for (usize probe = idx; ; probe = (probe + 1) % cap) {
			Bucket& b = this->buffer[probe];
			if (b.occupied) {
				if (b.key == key && !b.dead) {
					return {&b, true};
				}
			}
			// found an empty spot
			else {
				return {&b, false};
			}
		}
		// should never happen probably hopefully maybe probably :)
		return {nullptr, false};
	}

	V& operator[](K key)
	{
		// operator[] is also used for putting crap :)
		this->check_grow();

		Pair<Bucket*, bool> bucket = this->find(key);
		// if it's empty, operator[] places some crap
		if (!bucket.right) {
			bucket.left->occupied = true;
			bucket.left->key = key;
			this->len++;
		}

		return bucket.left->value;
	}

	// If true, the hashmap has that key. Useful because the `[]` operator automatically inserts an item if
	// it's not there.
	bool contains(K key)
	{
		return this->find(key).right;
	}

	// Removes the key from the hashmap. Returns true if the key is was found, returns false otherwise.
	bool remove(K key)
	{
		Pair<Bucket*, bool> bucket = this->find(key);
		// you can't kill someone that hasn't been born
		// don't quote me on this
		if (bucket.right) {
			bucket.left->dead = true;
		}

		return bucket.right;
	}

	// Returns how many items the hashmap currently has
	usize length() { return this->len; }
	// Returns the total amount of items the hashmap can currently hold (it'll grow when it's 50% full)
	usize capacity() { return this->cap; }

	// fucking iterator
	class Iterator {
	public:
		Iterator(Bucket* buffer, usize idx, usize cap) : buffer(buffer), idx(idx), cap(cap)
		{
			this->advance_to_valid();
		}

		Pair<K, V> operator*() const
		{
			Bucket& b = this->buffer[this->idx];
			return {b.key, b.value};
		}

		Iterator& operator++()
		{
			this->idx++;
			this->advance_to_valid();
			return *this;
		}

		bool operator!=(const Iterator& other) const
		{
			return this->idx != other.idx;
		}

	private:
		Bucket* buffer;
		usize idx;
		usize cap;

		void advance_to_valid()
		{
			// god
			while (this->idx < this->cap && (!this->buffer[this->idx].occupied || this->buffer[this->idx].dead)) {
				this->idx++;
			}
		}
	};

	Iterator begin() const { return Iterator(this->buffer, 0, this->cap); }
	Iterator end()   const { return Iterator(this->buffer + this->cap, this->cap, this->cap); }
};

// TODO pick one you dastardly scoundrel

template<typename K, typename V, uint64 (*HashFunc)(K key), usize InitialCapacity>
using ProLevelHashMap = AdvancedHashMap<K, V, HashFunc, InitialCapacity>;

template<typename K, typename V, uint64 (*HashFunc)(K key), usize InitialCapacity>
using TantalizingHashMap = AdvancedHashMap<K, V, HashFunc, InitialCapacity>;

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