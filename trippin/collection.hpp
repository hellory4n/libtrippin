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

#include "common.hpp"
#include "memory.hpp"
#include "string.hpp"
#include "utility.hpp"

namespace tr {

// Hashes an array of bytes, which is useful if you need to hash an array of bytes. Implemented with xxHash3
// 64-bits
uint64 hash(tr::Array<uint8> array);

// internal don't use probably :)
template<typename K>
uint64 __default_hash_function(const K& key)
{
	return tr::hash(Array<uint8>(reinterpret_cast<uint8*>(&key), sizeof(K)));
}
// internal don't use probably :)
template<>
inline uint64 __default_hash_function<String>(const String& key)
{
	return tr::hash(Array<uint8>(reinterpret_cast<uint8*>(key.buf()), key.len()));
}

// Useful for when you need *advanced* hashmaps
template<typename K>
struct HashMapSettings
{
	float64 load_factor;
	usize initial_capacity;
	uint64 (*hash_func)(const K& key);
};

// ahahsmhap :DD if you're interested this works with open addressing and linear probing, i'll probably
// change it if my brain expands to megamind levels of brain
template<typename K, typename V>
class HashMap
{
	static constexpr HashMapSettings<K> DEFAULT_SETTINGS = {0.5, 256, tr::__default_hash_function};

	struct Bucket
	{
		K key;
		V value;
		bool occupied;
		bool dead;
	};

	HashMapSettings<K> settings;

	Arena* arena = nullptr;
	Bucket* buffer = nullptr;

	// occupied includes removed keys, length doesn't
	usize occupied = 0;
	usize length = 0;
	usize capacity = 0;

public:
	explicit HashMap(Arena& arena, HashMapSettings<K> setting) : settings(setting), arena(&arena)
	{
		this->capacity = this->settings.initial_capacity;
		this->buffer = reinterpret_cast<Bucket*>(
			this->arena->alloc(this->settings.initial_capacity * sizeof(Bucket))
		);
	}

	explicit HashMap(Arena& arena) : HashMap(arena, DEFAULT_SETTINGS) {}

	// man fuck you
	HashMap() {}

	// Returns the index based on a key. That's how hash maps work.
	usize get_index(const K& key)
	{
		return this->settings.hash_func(key) % this->capacity;
	}

	void grow()
	{
		usize old_cap = this->capacity;
		this->capacity *= 2;
		Bucket* new_buffer = reinterpret_cast<Bucket*>(
			this->arena->alloc(this->capacity * sizeof(Bucket))
		);

		// changing the capacity fucks with the hashing
		// so we have to copy everything to new indexes
		for (usize i = 0; i < old_cap; i++) {
			Bucket& old_bucket = buffer[i];
			if (!old_bucket.occupied) continue;

			usize idx = this->get_index(old_bucket.key);

			// linear probe with wrap‑around
			for (usize probe = idx; ; probe = (probe + 1) % this->capacity) {
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

		this->buffer = new_buffer;
	}

	// Checks how full the hashmap is and resizes if necessary
	void check_grow()
	{
		float64 used = static_cast<float64>(this->occupied) / this->capacity;
		if (used <= this->settings.load_factor) {
			return;
		}
		this->grow();
	}

	// Returns the bucket and whether it's occupied (true for occupied, false for empty)
	Pair<Bucket*, bool> find(const K& key)
	{
		usize idx = this->get_index(key);
		for (usize probe = idx; ; probe = (probe + 1) % capacity) {
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

	V& operator[](const K& key)
	{
		// operator[] is also used for putting crap :)
		this->check_grow();

		Pair<Bucket*, bool> bucket = this->find(key);
		// if it's empty, operator[] places some crap
		if (!bucket.right) {
			bucket.left->occupied = true;
			bucket.left->key = key;
			this->occupied++;
			this->length++;
		}

		return bucket.left->value;
	}

	// If true, the hashmap has that key. Useful because the `[]` operator automatically inserts an item if
	// it's not there.
	bool contains(const K& key)
	{
		return this->find(key).right;
	}

	// Removes the key from the hashmap. Returns true if the key is was found, returns false otherwise.
	bool remove(const K& key)
	{
		Pair<Bucket*, bool> bucket = this->find(key);
		// you can't kill someone that hasn't been born
		// don't quote me on this
		if (bucket.right) {
			bucket.left->dead = true;
			this->length--;
		}

		return bucket.right;
	}

	// Returns how many items the hashmap currently has
	usize len() { return this->length; }
	// Returns the total amount of items the hashmap can currently hold (it'll grow when it's 50% full)
	usize cap() { return this->capacity; }

	// fucking iterator
	class Iterator {
	public:
		Iterator(Bucket* buffer, usize idx, usize cap) : buffer(buffer), idx(idx), cap(cap)
		{
			this->advance_to_valid();
		}

		Pair<K&, V&> operator*() const
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

	Iterator begin() const { return Iterator(this->buffer, 0, this->capacity); }
	Iterator end()   const { return Iterator(this->buffer + this->capacity, this->capacity, this->capacity); }
};

// TODO HashSet<T>, Stack<T>, Queue<T>, LinkedList<T>

}

#endif
