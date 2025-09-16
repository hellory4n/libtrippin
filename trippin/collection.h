/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/collection.h
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

#include <functional>
#include <utility>

#include "trippin/common.h"
#include "trippin/memory.h"
#include "trippin/string.h"

namespace tr {

// Hashes an array of bytes, which is useful if you need to hash an array of bytes. Implemented with
// 64-bit FNV-1a
uint64 hash(tr::Array<const uint8> array);

// internal don't use probably :)
template<typename K>
uint64 _default_hash_function(const K& key)
{
	return tr::hash(Array<const uint8>(reinterpret_cast<const uint8*>(&key), sizeof(K)));
}
// internal don't use probably :)
template<>
inline uint64 _default_hash_function<String>(const String& key)
{
	return tr::hash(Array<const uint8>(reinterpret_cast<const uint8*>(*key), key.len()));
}

// Useful for when you need *advanced* hashmaps
template<typename K>
struct HashMapSettings
{
	float64 load_factor;
	usize initial_capacity;
	uint64 (*hash_func)(const K& key);
};

// ahahsmhap :DD if you're interested this works with open addressing and linear probing, i'll
// probably change it if my brain expands to megamind levels of brain
template<typename K, typename V>
class HashMap
{
	static constexpr HashMapSettings<K> DEFAULT_SETTINGS = {
		0.5, 256, tr::_default_hash_function
	};

	struct Bucket
	{
		RefWrapper<K> key;
		RefWrapper<V> value;
		bool occupied;
		bool dead;
	};

	HashMapSettings<K> settings;

	Arena* src_arena = nullptr;
	Bucket* buffer = nullptr;

	// occupied includes removed keys, length doesn't
	usize occupied = 0;
	usize length = 0;
	usize capacity = 0;

public:
	using KeyType = K;
	using ValueType = V;

	explicit HashMap(Arena& arena, HashMapSettings<K> setting)
		: settings(setting)
		, src_arena(&arena)
	{
		this->capacity = this->settings.initial_capacity;
		this->buffer = static_cast<Bucket*>(
			this->src_arena->alloc(this->settings.initial_capacity * sizeof(Bucket))
		);
	}

	explicit HashMap(Arena& arena)
		: HashMap(arena, DEFAULT_SETTINGS)
	{
	}

	// man fuck you
	HashMap() { }

	// Returns the index based on a key. That's how hash maps work.
	usize get_index(K key)
	{
		return this->settings.hash_func(key) % this->capacity;
	}

	void grow()
	{
		usize old_cap = this->capacity;
		this->capacity *= 2;
		Bucket* new_buffer = static_cast<Bucket*>(
			this->src_arena->alloc(this->capacity * sizeof(Bucket))
		);

		// changing the capacity fucks with the hashing
		// so we have to copy everything to new indexes
		for (usize i = 0; i < old_cap; i++) {
			Bucket& old_bucket = buffer[i];
			if (!old_bucket.occupied) {
				continue;
			}

			usize idx = this->get_index(old_bucket.key);

			// linear probe with wrap‑around
			for (usize probe = idx;; probe = (probe + 1) % this->capacity) {
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
		// TODO you don't need a float here
		float64 used =
			static_cast<float64>(this->occupied) / static_cast<float64>(this->capacity);

		if (used <= this->settings.load_factor) {
			return;
		}
		this->grow();
	}

	// Returns the bucket and whether it's occupied (true for occupied, false for empty)
	Pair<Bucket*, bool> find(K key)
	{
		usize idx = this->get_index(key);
		for (usize probe = idx;; probe = (probe + 1) % capacity) {
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
		TR_UNREACHABLE();
	}

	V& operator[](K key)
	{
		// operator[] is also used for putting crap :)
		this->check_grow();

		// TODO put a _ before private members ffs
		auto [bucket, b_occupied] = this->find(key);
		// if it's empty, operator[] places some crap
		if (!b_occupied) {
			bucket->occupied = true;
			bucket->key = key;
			this->occupied++;
			this->length++;
		}

		return bucket->value;
	}

	// If true, the hashmap has that key. Useful because the `[]` operator automatically inserts
	// an item if it's not there.
	bool contains(K key)
	{
		return this->find(key).right;
	}

	// Removes the key from the hashmap. Returns true if the key is was found, returns false
	// otherwise.
	bool remove(K key)
	{
		auto [bucket, b_occupied] = this->find(key);
		// you can't kill someone that doesn't exist
		// don't quote me on this
		if (b_occupied) {
			bucket->dead = true;
			this->length--;
		}

		return b_occupied;
	}

	// Returns how many items the hashmap currently has
	usize len()
	{
		return this->length;
	}

	// Returns the total amount of items the hashmap can currently hold (it'll grow when it's
	// 50% full)
	usize cap()
	{
		return this->capacity;
	}

	// fucking iterator
	class Iterator
	{
	public:
		Iterator(Bucket* buf, usize index, usize capacity)
			: buffer(buf)
			, idx(index)
			, cap(capacity)
		{
			this->find_valid();
		}

		Pair<K&, V&> operator*() const
		{
			Bucket& b = this->buffer[this->idx];
			return {b.key, b.value};
		}

		Iterator& operator++()
		{
			this->idx++;
			this->find_valid();
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

		void find_valid()
		{
			// god
			while (this->idx < this->cap && (!this->buffer[this->idx].occupied ||
							 this->buffer[this->idx].dead)) {
				this->idx++;
			}
		}
	};

	Iterator begin() const
	{
		return Iterator(this->buffer, 0, this->capacity);
	}

	Iterator end() const
	{
		return Iterator(this->buffer + this->capacity, this->capacity, this->capacity);
	}
};

// I sure love events signals whatever. The reason you're supposed to use this instead of a function
// pointer/`std::function` is that this can have multiple listeners, which is probably important.
template<typename... Args>
class Signal
{
	using SignalFunc = std::function<void(Args...)>;
	Array<SignalFunc> listeners;

public:
	Signal(Arena& arena)
		: listeners(arena)
	{
	}

	// Adds a listener to the signal :)
	void connect(SignalFunc func)
	{
		this->listeners.add(func);
	}

	// TODO disconnect()

	// Emits the signal to all listeners that's what a signal does lmao.
	void emit(Args&&... args)
	{
		for (auto [_, func] : this->listeners) {
			func(std::forward<Args>(args)...);
		}
	}
};

// I sure love events signals whatever. The reason you're supposed to use this instead of a function
// pointer/`std::function` is that this can have multiple listeners, which is probably important.
template<>
class Signal<void>
{
	using SignalFunc = std::function<void(void)>;
	Array<SignalFunc> listeners;

public:
	Signal(Arena& arena)
		: listeners(arena)
	{
	}

	// Adds a listener to the signal :)
	void connect(SignalFunc func)
	{
		this->listeners.add(func);
	}

	// TODO disconnect()

	// Emits the signal to all listeners that's what a signal does lmao.
	void emit()
	{
		for (auto [_, func] : this->listeners) {
			func();
		}
	}
};

// TODO HashSet<T>, Stack<T>, Queue<T>, LinkedList<T>

}

#endif
