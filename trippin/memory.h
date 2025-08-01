/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/memory.h
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

#ifndef _TRIPPIN_MEMORY_H
#define _TRIPPIN_MEMORY_H

// clangd are you stupid
#include <new> // IWYU pragma: keep
#include <initializer_list>
#include <utility>
#include <string.h>

#include "trippin/common.h"
#include "trippin/log.h"

namespace tr {

// Why the fuck not.
struct MemoryInfo {
	// Currently allocated by arenas, in bytes
	isize allocated = 0;
	// Like `allocated`, but cumulative
	isize cumulative_allocated = 0;
	// Amount of alive reference counted objects
	isize ref_counted_objs = 0;
	// Like `ref_counted_objs`, but cumulative
	isize cumulative_ref_counted_objs = 0;
	// Total amount of memory freed from arenas, in bytes
	isize freed_by_arenas = 0;
	// Amount of freed reference counted objects
	isize freed_ref_counted_objs = 0;
	// Total amount of alive arena pages
	isize alive_pages = 0;
	// All arena pages that have ever existed
	isize cumulative_pages = 0;
	// Total amount of freed arena pages
	isize freed_pages = 0;
};

// As the name implies, it gets the memory info. Idk why.
[[deprecated("this api is just kinda crap, will be removed in v2.5")]]
MemoryInfo get_memory_info();

// Converts kilobytes to bytes
static constexpr usize kb_to_bytes(usize x) { return x * 1024; }
// Converts megabytes to bytes
static constexpr usize mb_to_bytes(usize x) { return kb_to_bytes(x) * 1024; }
// Converts gigabytes to bytes
static constexpr usize gb_to_bytes(usize x) { return mb_to_bytes(x) * 1024; }

// Converts bytes to kilobytes
static constexpr usize bytes_to_kb(usize x) { return x / 1024; }
// Converts bytes to megabytes
static constexpr usize bytes_to_mb(usize x) { return bytes_to_kb(x) / 1024; }
// Converts bytes to gigabytes
static constexpr usize bytes_to_gb(usize x) { return bytes_to_mb(x) / 1024; }

// Arenas are made of many buffers.
class ArenaPage
{
public:
	const usize bufsize = 0;
	const usize alignment = 0;
	usize alloc_pos = 0;
	ArenaPage* prev = nullptr;
	ArenaPage* next = nullptr;
	void* buffer = nullptr;

	explicit ArenaPage(usize size, usize align = alignof(max_align_t));
	~ArenaPage();

	// Returns how much space left the page has
	usize available_space() const;

	// Allocates something in the page lmao. Returns null on failure
	void* alloc(usize size, usize align);
};

// Internal utility to manage calling destructors :)
struct DestructorCall
{
	void (*func)(void* obj);
	void* object;
	DestructorCall* next;
};

// Life changing allocator.
class Arena
{
	usize page_size = 0;
	// stupid names bcuz the functions are `capacity` and `allocated`
	usize bytes_capacity = 0;
	usize bytes_allocated = 0;
	ArenaPage* page = nullptr;
	DestructorCall* destructors = nullptr;

	void call_destructors();

public:
	// :)
	Arena() : Arena(tr::kb_to_bytes(64)) {}

	// Initializes the arena. `page_size` is the base size for the buffers, you can have more buffers or
	// bigger buffers.
	explicit Arena(usize pg_size);

	// Frees the arena.
	~Arena();

	// Allocates some crap on the arena.
	[[gnu::malloc]]
	void* alloc(usize size, usize align = alignof(max_align_t));

	// Reuses the entire arena and sets everything to 0 :)
	void reset();

	// Kinda like `new`/`malloc` but for arenas. The funky variadic templates allow you to pass any arguments
	// here to the actual constructor. Note this supports calling destructors for when the arena is deleted,
	// but why?
	template<typename T, typename... Args>
	T& make(Args&&... args)
	{
		void* baseball = this->alloc(sizeof(T), alignof(T));
		T* huh = new (baseball) T(std::forward<Args>(args)...);

		// fancy fuckery to get destructors to be called :)
		auto* call = static_cast<DestructorCall*>(this->alloc(sizeof(DestructorCall)));
		call->func = [](void* obj) -> void {
			static_cast<T*>(obj)->~T();
		};
		call->object = huh;
		call->next = this->destructors;
		this->destructors = call;

		return *huh;
	}

	// Returns how much has already been allocated in the arena, in bytes.
	usize allocated() const;

	// Returns how many bytes the arena can hold before expanding, in bytes.
	usize capacity() const;
};

// Temporary arena intended for temporary allocations. In other words, a sane `alloca()`.
Arena& scratchpad();

// This is just for iterators
template<typename T>
struct ArrayItem
{
	usize i;
	T& val;
};

// A slice of memory, usually from an arena but can point to anywhere. Similar to a Go slice, or other
// examples. Arrays don't own the value and don't use fancy RAII fuckery, so you can pass them by value.
template<typename T>
class Array
{
	// used for when you don't set the length (which you usually do if you're just gonna use .add())
	static constexpr usize INITIAL_CAPACITY = 16;

	T* ptr = nullptr;
	Arena* src_arena = nullptr;
	usize length = 0;
	usize capacity = 0;

public:
	// Initializes an empty array at an arena.
	explicit Array(Arena& arena, usize len) : src_arena(&arena), length(len), capacity(len)
	{
		// you may initialize with a length of 0 so you can then add crap later
		// i'm just keeping this behavior so it doesn't break everything that used Array(arena, 0)
		if (len == 0) {
			this->length = 0;
			this->capacity = INITIAL_CAPACITY;
		}

		this->ptr = static_cast<T*>(arena.alloc(sizeof(T) * this->capacity));
	}

	// Initializes an array from a buffer. (the data is copied into the arena)
	explicit Array(Arena& arena, T* data, usize len) : src_arena(&arena), length(len), capacity(len)
	{
		// you may initialize with a length of 0 so you can then add crap later
		// i'm just keeping this behavior so it doesn't break everything :)
		if (len == 0) {
			this->length = 0;
			this->capacity = INITIAL_CAPACITY;
		}

		this->ptr = static_cast<T*>(arena.alloc(sizeof(T) * this->capacity));
		if (len == 0) return;

		// 'void* memcpy(void*, const void*, size_t)' forming offset [1, 1024] is out of the bounds [0, 1]
		// the warning is wrong :)
		#ifdef TR_ONLY_GCC
		TR_GCC_IGNORE_WARNING(-Warray-bounds);
		TR_GCC_IGNORE_WARNING(-Wstringop-overread);
		#endif
		memcpy(static_cast<void*>(this->ptr), data, len * sizeof(T));
		#ifdef TR_ONLY_GCC
		TR_GCC_RESTORE();
		TR_GCC_RESTORE();
		#endif
	}

	// Initializes an array that points to any buffer. You really should only use this for temporary arrays.
	constexpr explicit Array(T* data, usize len) : ptr(data), src_arena(nullptr), length(len), capacity(len) {}

	// why bjarne stroustrup why can't i make this myself why is std::initializer_list<T> special i know
	// this is from c++11 but i don't care i'm gonna blame bjarne stroustrup inventor of C incremented
	constexpr Array(std::initializer_list<T> initlist) : src_arena(nullptr)
	{
		this->capacity = initlist.size();
		this->length = initlist.size();
		// may you please shut the fuck up fucking hell man no one loves you🥰🥰🥰🥰
		this->ptr = const_cast<T*>(initlist.begin());
	}

	explicit Array(Arena& arena, std::initializer_list<T> initlist) :
		Array(arena, const_cast<T*>(initlist.begin()), initlist.size()) {}

	// man fuck you
	constexpr Array() : ptr(nullptr), src_arena(nullptr), length(0), capacity(0) {}

	// Initializes the array with just an arena so you can add crap later :)
	explicit Array(Arena& arena) : Array(arena, 0) {}

	constexpr T& operator[](usize idx) const
	{
		if (idx >= this->length) {
			tr::panic("index out of range: %zu in an array of %zu", idx, this->length);
		}
		return this->ptr[idx];
	}

	// Similar to `operator[]`, but when getting an index out of bounds, instead of panicking, it returns
	// null, which is probably useful sometimes.
	constexpr MaybePtr<T> try_get(usize idx) const
	{
		if (idx >= this->length) {
			return {};
		}
		return &this->ptr[idx];
	}

	// Returns the buffer.
	constexpr T* buf() const       { return this->ptr; }
	// Returns the length of the array.
	constexpr usize len() const    { return this->length; }
	// Returns how many items the array can hold before having to resize.
	constexpr usize cap() const    { return this->capacity; }
	// Shorthand for `.buf()`
	constexpr T* operator*() const { return this->buf(); }

	// fucking iterator
	class Iterator {
	public:
		constexpr Iterator(T* pointer, usize index) : idx(index), ptr(pointer) {}
		constexpr ArrayItem<T> operator*() const { return {this->idx, *this->ptr}; }
		constexpr Iterator& operator++() { this->ptr++; this->idx++; return *this; }
		constexpr bool operator!=(const Iterator& other) const { return ptr != other.ptr; }
	private:
		usize idx;
		T* ptr;
	};

	constexpr Iterator begin() const { return Iterator(this->buf(), 0); }
	constexpr Iterator end()   const { return Iterator(this->buf() + this->len(), this->len()); }

	// Adds a new item to the array, and resizes it if necessary. This only works on arena-allocated arrays,
	// if you try to use this on an array without an arena, it will panic.
	void add(const T& val)
	{
		if (this->src_arena == nullptr) {
			tr::panic("resizing arena-less tr::Array<T> is not allowed");
		}

		// does it already fit?
		if (this->length < this->capacity) {
			(*this)[this->length++] = val;
			return;
		}

		// reallocate array
		T* old_buffer = this->ptr;
		this->capacity *= 2;
		this->ptr = static_cast<T*>(src_arena->alloc(this->capacity * sizeof(T)));
		// you may initialize with a length of 0 so you can then add crap later
		if (this->length > 0) {
			memcpy(static_cast<void*>(this->ptr), static_cast<const void*>(old_buffer),
				this->length * sizeof(T));
		}

		(*this)[this->length++] = val;
	}

	// As the name implies, it copies the array and its items to somewhere else.
	Array<T> duplicate(Arena& arena) const
	{
		Array<T> result(arena, this->len());
		memcpy(result.buf(), this->buf(), this->len() * sizeof(T));
		return result;
	}
};

}

#endif
