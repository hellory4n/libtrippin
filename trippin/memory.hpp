/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/memory.hpp
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

#include <string.h>

#include "common.hpp"

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
MemoryInfo get_memory_info();

// Implements reference counting through inheritance. Note you have to wrap your values in a `tr::Ref<T>`
// so it's not esoteric to use.
class RefCounted
{
	mutable isize count = 0;

public:
	RefCounted() : count(0) {}
	virtual ~RefCounted() {}

	void retain() const;
	void release() const;
};

// Non-esoteric wrapper around `tr::RefCounted`. It also allows null, if you don't want that to happen, use
// `tr::Ref<T>`
template<typename T>
class MaybeRef;

// Non-esoteric wrapper around `tr::RefCounted`. It also panics on null, if you don't want that to happen, use
// `tr::MaybeRef<T>`
template<typename T>
class Ref
{
	T* ptr = nullptr;
	// man
	friend class MaybeRef<T>;

	RefCounted* refcounted() const { return dynamic_cast<RefCounted*>(ptr); }

public:
	// C++ can be annoying
	Ref() : ptr(nullptr) {}

	Ref(T* ptr) : ptr(ptr)
	{
		if (this->ptr == nullptr) {
			tr::panic("tr::Ref<T> can't be null, if that's intentional use tr::MaybeRef<T>");
		}
		this->refcounted()->retain();
	}

	Ref(const Ref& ref) : ptr(ref.ptr)
	{
		if (this->ptr == nullptr) {
			tr::panic("tr::Ref<T> can't be null, if that's intentional use tr::MaybeRef<T>");
		}
		this->refcounted()->retain();
	}

	Ref(const MaybeRef<T>& ref);

    Ref(Ref&& other) : ptr(other.ptr) { other.ptr = nullptr; }

	~Ref()
	{
		if (this->ptr == nullptr) return;
		this->refcounted()->release();
	}

	Ref& operator=(T* p)
	{
		if (p == nullptr) {
			tr::panic("tr::Ref<T> can't be null, if that's intentional use tr::MaybeRef<T>");
		}
		if (this->ptr != nullptr) this->refcounted()->retain();
		if (this->ptr != nullptr) this->refcounted()->release();
		this->ptr = p;
		return *this;
	}

	Ref& operator=(const Ref<T>& other)
	{
		if (this != &other) {
			if (other.ptr != nullptr) other.refcounted()->retain();
			if (this->ptr != nullptr) {
				this->refcounted()->release();
			}
			this->ptr = other.ptr;
		}
		return *this;
	}

	Ref& operator=(Ref<T>&& other)
	{
		if (this != &other) {
			if (this->ptr != nullptr) {
				this->refcounted()->release();
			}
			ptr = other.ptr;
			other.ptr = nullptr;
		}
		return *this;
	}

	// Returns the crap pointer.
	T* get() const
	{
		if (this->ptr == nullptr) {
			tr::panic("tr::Ref<T> is null, if that's intentional use tr::MaybeRef<T>");
		}
		return this->ptr;
	}

	// help
	T* operator->() const                   { return this->get(); }
	T& operator*() const                    { return *this->get(); }
	operator T*() const                     { return this->get(); }
	bool operator==(const MaybeRef<T>& ref) { return this->ptr == ref.ptr; }
	bool operator==(const Ref<T>& ref)      { return this->ptr == ref.ptr; }
	bool operator==(const T* p)             { return this->ptr == p; }
	bool operator!=(const MaybeRef<T>& ref) { return this->ptr != ref.ptr; }
	bool operator!=(const Ref<T>& ref)      { return this->ptr != ref.ptr; }
	bool operator!=(const T* p)             { return this->ptr != p; }
};

// Non-esoteric wrapper around `tr::RefCounted`. It also allows null, if you don't want that to happen, use
// `tr::Ref<T>`
template<typename T>
class MaybeRef
{
	T* ptr = nullptr;
	// man
	friend class Ref<T>;

	RefCounted* refcounted() const
	{
		if (this->ptr == nullptr) tr::panic("MaybeRef<T> is null (likely a library error)");
		RefCounted* fukc = dynamic_cast<RefCounted*>(this->ptr);
		if (fukc == nullptr) {
			tr::panic("couldn't cast type to tr::RefCounted, to use tr::MaybeRef<T> your types must inherit tr::RefCounted");
		}
		return fukc;
	}

public:
	MaybeRef(T* ptr = nullptr) : ptr(ptr)
	{
		if (this->ptr == nullptr) return;
		this->refcounted()->retain();
	}

	MaybeRef(const MaybeRef& ref) : ptr(ref.ptr)
	{
		if (this->ptr == nullptr) return;
		this->refcounted()->retain();
	}

	MaybeRef(const Ref<T>& ref) : ptr(ref.ptr)
	{
		if (this->ptr == nullptr) return;
		this->refcounted()->retain();
	}

	MaybeRef(MaybeRef&& other) : ptr(other.ptr) { other.ptr = nullptr; }

	~MaybeRef()
	{
		if (this->ptr == nullptr) return;
		this->refcounted()->release();
	}

	MaybeRef& operator=(T* p)
	{
		if (p != nullptr) {
			this->refcounted()->retain();
		}
		if (this->ptr != nullptr) {
			this->refcounted()->release();
		}
		this->ptr = p;
		return *this;
	}

	MaybeRef& operator=(const MaybeRef<T>& other)
	{
		if (this != &other) {
			if (other.ptr != nullptr) other.refcounted()->retain();
			if (this->ptr != nullptr) this->refcounted()->release();
			this->ptr = other.ptr;
		}
		return *this;
	}

	MaybeRef& operator=(MaybeRef<T>&& other)
	{
		if (this != &other) {
			if (this->ptr != nullptr) {
				this->refcounted()->release();
			}
			ptr = other.ptr;
			other.ptr = nullptr;
		}
		return *this;
	}

	// Returns the crap pointer.
	T* get() const
	{
		return this->ptr;
	}

	// help
	T* operator->() const                   { return this->ptr; }
	T& operator*() const                    { return *this->ptr; }
	operator T*() const                     { return this->ptr; }
	bool operator==(const MaybeRef<T>& ref) { return this->ptr == ref.ptr; }
	bool operator==(const Ref<T>& ref)      { return this->ptr == ref.ptr; }
	bool operator==(const T* p)             { return this->ptr == p; }
	bool operator!=(const MaybeRef<T>& ref) { return this->ptr != ref.ptr; }
	bool operator!=(const Ref<T>& ref)      { return this->ptr != ref.ptr; }
	bool operator!=(const T* p)             { return this->ptr != p; }
};

// man
template<typename T> Ref<T>::Ref(const MaybeRef<T>& other) : ptr(other.ptr)
{
    if (!this->ptr) {
        tr::panic("can't convert null tr::MaybeRef<T> to tr::Ref<T>");
	}
    this->refcounted()->retain();
}

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
	usize size = 0;
	usize alloc_pos = 0;
	ArenaPage* prev = nullptr;
	ArenaPage* next = nullptr;
	void* buffer = nullptr;

	explicit ArenaPage(usize size);
	~ArenaPage();

	// Returns how much space left the page has
	usize available_space();
};

// Life changing allocator.
class Arena : public RefCounted
{
public:
	usize page_size = 0;
	ArenaPage* page = nullptr;

	// This is just for the compiler to shut up
	Arena() : page_size(0), page(nullptr) {}

	// Initializes the arena. `page_size` is the base size for the buffers, you can have more buffers or
	// bigger buffers.
	explicit Arena(usize page_size);

	// Frees the arena. Note this doesn't call any destructors from structs you may have allocated, as I
	// don't know how to do that.
	~Arena();

	// Allocates some crap on the arena.
	void* alloc(usize size);

	// Literally just `Arena::alloc()` but for structs and crap. Note this doesn't call the constructor, it
	// just allocates enough space for that type. If you want to use arenas on your own types, you're supposed
	// to pass it in the constructor (e.g. `CrapClass(tr::Ref<tr::Arena> arena)`)
	template<typename T> T* alloc()
	{
		return reinterpret_cast<T*>(this->alloc(sizeof(T)));
	}

	// Makes sure there's enough space to fit `size`. Useful for when you're about to allocate a lot of
	// objects and don't want it to try to figure out the pages 57399593895 times.
	void prealloc(usize size);
};

// This is just for iterators
template<typename T>
struct ArrayItem
{
	usize i;
	T& val;
};

// A slice of memory, usually from an arena but can point to anywhere. Similar to a Go slice, or other
// examples.
template<typename T>
class Array
{
	// i'm gonna keep the arena when i remove .add() so that for as long as the array exists, the arena exists too
	// it's an ownership thing yknow
	MaybeRef<Arena> src_arena = nullptr;
	T* ptr = nullptr;
	usize len = 0;
	usize cap = 0;

public:
	// Initializes an empty array at an arena.
	explicit Array(Ref<Arena> arena, usize len) : src_arena(arena), len(len), cap(len)
	{
		// you may initialize with a length of 0 so you can then add crap
		if (len > 0) {
			this->ptr = reinterpret_cast<T*>(arena->alloc(sizeof(T) * len));
		}
	}

	// Initializes an array from a buffer. (the data is copied into the arena)
	explicit Array(Ref<Arena> arena, T* data, usize len) : src_arena(arena), len(len), cap(len)
	{
		this->ptr = reinterpret_cast<T*>(arena->alloc(sizeof(T) * len));
		// 'void* memcpy(void*, const void*, size_t)' forming offset [1, 1024] is out of the bounds [0, 1]
		// the warning is wrong :)
		#ifdef TR_ONLY_GCC
		TR_GCC_IGNORE_WARNING(-Warray-bounds);
		TR_GCC_IGNORE_WARNING(-Wstringop-overread);
		#endif
		memcpy(reinterpret_cast<void*>(this->ptr), data, len * sizeof(T));
		#ifdef TR_ONLY_GCC
		TR_GCC_RESTORE();
		TR_GCC_RESTORE();
		#endif
	}

	// Initializes an array that points to any buffer. You really should only use this for temporary arrays.
	explicit Array(T* data, usize len) : src_arena(nullptr), ptr(data), len(len), cap(len) {}

	// man fuck you
	Array() : src_arena(nullptr), ptr(nullptr), len(0), cap(0) {}

	T& operator[](usize idx) const
	{
		if (idx >= this->len) {
			tr::panic("index out of range: %zu in an array of %zu", idx, this->len);
		}
		return this->ptr[idx];
	}

	// Returns the buffer.
	T* buffer() const      { return this->ptr; }
	// Returns the length of the array.
	usize length() const   { return this->len; }
	// Returns how many items the array can hold before having to resize.
	[[deprecated("this will soon be removed along with .add()")]] usize capacity() const { return this->cap; }

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
	Iterator end()   const { return Iterator(this->buffer() + this->len, this->len); }

	// Adds a new item to the array, and resizes it if necessary. This only works on arena-allocated arrays,
	// if you try to use this on an array without an arena, it will panic.
	[[deprecated("use tr::List<T> instead, .add() will be removed in v2.3")]] void add(T val)
	{
		if (this->src_arena == nullptr) {
			tr::panic("resizing arena-less tr::Array<T> is not allowed");
		}

		// does it already fit?
		if (this->len + 1 <= this->cap) {
			(*this)[this->len++] = val;
			return;
		}

		// reallocate array
		// TODO use pages to not waste so much memory
		T* old_buffer = this->ptr;
		this->cap *= 2;
		this->ptr = reinterpret_cast<T*>(this->src_arena->alloc(this->cap * sizeof(T)));
		// you may initialize with a length of 0 so you can then add crap
		if (this->len > 0) {
			memcpy(this->ptr, old_buffer, this->len * sizeof(T));
		}

		(*this)[this->len++] = val;
	}

	// As the name implies, it copies the array and its items to somewhere else.
	Array<T> duplicate(Ref<Arena> arena) const
	{
		Array<T> result(arena, this->length());
		memcpy(result.buffer(), this->buffer(), this->length() * sizeof(T));
		return result;
	}
};

}

#endif
