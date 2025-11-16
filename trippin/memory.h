/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/memory.h
 * Arenas, arrays, and a few utilities
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

#include <initializer_list>
#include <new> // IWYU pragma: keep
#include <type_traits>
#include <utility>

#include "trippin/common.h"
#include "trippin/log.h"
#include "trippin/math.h"

namespace tr {

// Converts kilobytes to bytes
static constexpr usize kb_to_bytes(usize x)
{
	return x * 1024;
}
// Converts megabytes to bytes
static constexpr usize mb_to_bytes(usize x)
{
	return tr::kb_to_bytes(x) * 1024;
}
// Converts gigabytes to bytes
static constexpr usize gb_to_bytes(usize x)
{
	return tr::mb_to_bytes(x) * 1024;
}

// Converts bytes to kilobytes
static constexpr usize bytes_to_kb(usize x)
{
	return x / 1024;
}
// Converts bytes to megabytes
static constexpr usize bytes_to_mb(usize x)
{
	return tr::bytes_to_kb(x) / 1024;
}
// Converts bytes to gigabytes
static constexpr usize bytes_to_gb(usize x)
{
	return tr::bytes_to_mb(x) / 1024;
}

// memcpy is evil and breaks vtables :(
template<typename T>
requires(!std::is_const_v<T>)
void _copy_items(RefWrapper<T>* dst, const RefWrapper<T>* src, usize len)
{
	for (usize i = 0; i < len; i++) {
		if constexpr (std::is_reference_v<T> || std::is_trivially_copyable_v<T>) {
			dst[i] = src[i];
		}
		else {
			new (&dst[i]) T(src[i]);
		}
	}
}

// Settings for an arena. How incredible.
struct ArenaSettings
{
	enum class ErrorBehavior : uint8
	{
		PANIC,
		RETURN_NULL
	};

	// Base size for the buffers, you can have more buffers or bigger buffers.
	usize page_size = tr::kb_to_bytes(4);
	// null = no limit (grows infinitely)
	Maybe<usize> max_pages = {};
	// If false, the pages will be left with random garbage (like malloc). Maybe you want that,
	// idk
	bool zero_initialize = true;
	// What should happen on allocation errors
	ErrorBehavior error_behavior = ArenaSettings::ErrorBehavior::PANIC;
};

// Arenas are made of many buffers.
class ArenaPage
{
public:
	const usize bufsize = 0;
	usize alloc_pos = 0;
	ArenaPage* prev = nullptr;
	ArenaPage* next = nullptr;
	void* buffer = nullptr;

	explicit ArenaPage(ArenaSettings settings, usize size);
	void free();

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

// An arena allocator that grows infinitely through pages. Good enough 90% of the time. Saucy.
class Arena
{
public:
	// :)
	Arena()
		: Arena(ArenaSettings{})
	{
	}

	explicit Arena(ArenaSettings settings);

	// Initializes the arena. `page_size` is the base size for the buffers, you
	// can have more buffers or bigger buffers.
	[[deprecated("use the ArenaSettings overload")]]
	explicit Arena(usize page_size)
		: Arena({.page_size = page_size})
	{
	}

	// Frees the arena.
	virtual void free();

	// Allocates some crap on the arena.
	[[nodiscard, gnu::malloc]]
	virtual void* alloc(usize size, usize align = alignof(max_align_t)) TR_LIFETIMEBOUND;

	// Like `.alloc()` but without an `static_cast<T*>`. Mind-boggling. You're required to use a
	// pointer so that it's not confused with `.make_ptr()`, this is the evil low-level version.
	template<typename T>
	requires std::is_pointer_v<T>
	[[nodiscard, gnu::malloc]]
	auto* alloc(usize size, usize align = alignof(std::remove_pointer_t<T>)) TR_LIFETIMEBOUND
	{
		return static_cast<T>(alloc(size, align));
	}

	// Reuses the entire arena and sets everything to 0 :)
	virtual void reset();

	// Kinda like `new`/`malloc` but for arenas. The funky variadic templates
	// allow you to pass any arguments here to the actual constructor. Note this
	// supports calling destructors for when the arena is deleted, but why?
	template<typename T, typename... Args>
	[[deprecated("use make_ref or make_ptr instead")]]
	T& make(Args&&... args) TR_LIFETIMEBOUND
	{
		void* baseball = this->alloc(sizeof(T), alignof(T));
		T* huh = new (baseball) T(std::forward<Args>(args)...);

		// fancy fuckery to get destructors to be called :)
		auto* call = static_cast<DestructorCall*>(this->alloc(sizeof(DestructorCall)));
		call->func = [](void* obj) -> void { static_cast<T*>(obj)->~T(); };
		call->object = huh;
		call->next = this->_destructors;
		this->_destructors = call;

		return *huh;
	}

	// Kinda like `new`/`malloc` but for arenas. The funky variadic templates
	// allow you to pass any arguments here to the actual constructor. Note this
	// supports calling destructors for when the arena is deleted, but why would you ever do
	// that
	template<typename T, typename... Args>
	[[nodiscard]]
	T& make_ref(Args... args) TR_LIFETIMEBOUND
	{
		void* ptr = this->alloc(sizeof(T), alignof(T));
		T* obj = new (ptr) T(std::forward<Args>(args)...);

		// fancy fuckery to get destructors to be called
		auto* call = static_cast<DestructorCall*>(this->alloc(sizeof(DestructorCall)));
		call->func = [](void* obj) { static_cast<T*>(obj)->~T(); };
		call->object = obj;
		call->next = this->_destructors;
		this->_destructors = call;

		return *obj;
	}

	// Kinda like `new`/`malloc` but for arenas. The funky variadic templates
	// allow you to pass any arguments here to the actual constructor. Note this
	// supports calling destructors for when the arena is deleted, but why would you ever do
	// that
	template<typename T, typename... Args>
	[[nodiscard]]
	T* make_ptr(Args... args) TR_LIFETIMEBOUND
	{
		void* ptr = this->alloc(sizeof(T), alignof(T));
		T* obj = new (ptr) T(std::forward<Args>(args)...);

		// fancy fuckery to get destructors to be called
		auto* call = static_cast<DestructorCall*>(this->alloc(sizeof(DestructorCall)));
		call->func = [](void* obj) { static_cast<T*>(obj)->~T(); };
		call->object = obj;
		call->next = this->_destructors;
		this->_destructors = call;

		return obj;
	}

	// Returns how much has already been allocated in the arena, in bytes.
	virtual usize allocated() const;

	// Returns how many bytes the arena can hold before expanding, in bytes.
	virtual usize capacity() const;

protected:
	// inheritance based as a bit of a hack so i don't have to change Arena& to Allocator& or
	// whatever everywhere, + all the allocators are arena-like anyway
	ArenaSettings _settings = {};
	usize _capacity = 0;
	usize _allocated = 0;
	usize _pages = 0;
	ArenaPage* _page = nullptr;
	DestructorCall* _destructors = nullptr;

	virtual bool _initialized() const;
	void _call_destructors();
};

// This is a page size (it can use multiple pages)
constexpr usize SCRATCH_BACKING_BUFFER_SIZE = tr::mb_to_bytes(4);

// Looks like an arena, can be passed as an arena, but is actually not really, instead it shares a
// buffer with other ScratchArenas so that it can be used for temporary allocations with pratically
// zero overhead. It works a lot like the stack, except it can't overflow.
class ScratchArena : public Arena
{
	// :)
	ScratchArena();

	// Frees the arena.
	void free() override;

	// Allocates some crap on the arena.
	[[nodiscard, gnu::malloc]]
	void* alloc(usize size, usize align = alignof(max_align_t)) TR_LIFETIMEBOUND override;

	// Does the same as freeing the arena then making a new one
	void reset() override;

	// Returns how much has already been allocated in the arena, in bytes.
	usize allocated() const override;

	// Returns the size of the underlying buffer. Note these can still expand (like a regular
	// arena does), you can allocate more than whatever this returns.
	usize capacity() const override;

private:
	byte* _start_alloc_pos = nullptr;
	byte* _end_alloc_pos = nullptr;
	ArenaPage* _start_page = nullptr;

	bool _initialized() const override
	{
		return _start_alloc_pos != nullptr;
	}
};

// Temporary arena intended for temporary allocations. In other words, a sane `alloca()`.
[[deprecated("use tr::ScratchArena")]]
Arena& scratchpad();

// This is just for iterators
template<typename T>
struct ArrayItem
{
	usize i;
	T val;
};

// std::initializer_list<T> doesn't live very long. to prevent fucking (dangling ptrs), we have to
// copy the data somewhere that lasts longer, and trap them in purgatory for as long as the program
// is open.
extern Arena _consty_arena;

// man
enum class ArrayClearBehavior
{
	RESET_ALL_ITEMS,
	DO_NOTHING
};

// used for when you don't set the length (which you usually do if you're just gonna use .add())
static constexpr usize ARRAY_INITIAL_CAPACITY = 16;

// A slice of memory, usually from an arena but can point to anywhere. Similar to a Go slice, or
// other examples. Arrays don't own the value and don't use fancy RAII faffery, so you can pass them
// by value. Arrays can be used both as a vector (`tr::Array<T>` ~= `std::vector<T>`), and, as an
// immutable view (`tr::Array<const T>` ~= `const T*` or `std::span<T>`)
template<typename T>
class Array
{
	// used for the ptr types
	using MutT = RefWrapper<std::remove_const_t<T>>;
	using ConstT = const RefWrapper<T>;

	// if it's from an arena the internal ptr just can't be const, as you're copying the
	// original const data to the arena BUT if it's not and is just pointing somewhere, then
	// having it not be const can be very questionable
	union {
		MutT* _arena_ptr = nullptr;
		ConstT* _ptr;
	};
	Arena* _src_arena = nullptr;
	usize _len = 0;
	usize _cap = 0;
	bool _can_grow;

	constexpr bool _is_from_arena() const
	{
		return _src_arena != nullptr;
	}

	constexpr void _validate() const
	{
		if (_ptr == nullptr) [[unlikely]] {
			tr::panic("uninitialized tr::Array<T>!");
		}
	}

public:
	using Type = T;

	// Initializes an empty array at an arena.
	Array(Arena& arena, usize len)
	requires(!std::is_const_v<T>) // otherwise what would you even do with it?
		: _src_arena(&arena)
		, _len(len)
		, _cap(len)
		, _can_grow(true)
	{
		// you may initialize with a length of 0 so you can then add crap later
		// i'm just keeping this behavior so it doesn't break everything that used
		// Array(arena, 0)
		if (len == 0) {
			_cap = ARRAY_INITIAL_CAPACITY;
		}

		_arena_ptr = static_cast<MutT*>(arena.alloc(sizeof(T) * _cap));

		// arena memory isn't always zero-initialized
		if constexpr (std::is_reference_v<T>) {
			for (usize i = 0; i < len; i++) {
				_arena_ptr[i] = nullptr;
			}
		}
		else {
			for (auto [_, item] : *this) {
				item = T{};
			}
		}
	}

	// Initializes an array from a buffer. (the data is copied into the arena)
	Array(Arena& arena, ConstT* data, usize len)
		: _src_arena(&arena)
		, _len(len)
		, _cap(len)
		, _can_grow(true)
	{
		TR_ASSERT_MSG(
			data != nullptr, "tr::Array<T> can't be null, if that's intentional use "
					 "tr::Maybe<tr::Array<T>>"
		);

		// you may initialize with a length of 0 so you can then add crap later
		// i'm just keeping this behavior so it doesn't break everything :)
		if (len == 0) {
			_len = 0;
			_cap = ARRAY_INITIAL_CAPACITY;
		}

		if (data == nullptr) {
			tr::panic(
				"tr::Array<T> can't be null (use tr::Maybe<T> if this is "
				"intentional)"
			);
		}

		_arena_ptr = static_cast<MutT*>(arena.alloc(sizeof(T) * _cap));
		if (len == 0) {
			return;
		}

		tr::_copy_items<MutT>(_arena_ptr, data, len);
	}

	// Initializes an array that points to any buffer. You really should only use
	// this for temporary arrays.
	constexpr Array(ConstT* data, usize len)
		: _ptr(data)
		, _len(len)
		, _cap(len)
		, _can_grow(false)
	{
		if (data == nullptr) {
			tr::panic(
				"tr::Array<T> can't be null (use tr::Maybe<T> if this is "
				"intentional)"
			);
		}
	}

	Array(Arena& arena, std::initializer_list<RefWrapper<const T>> initlist)
		: Array(arena, initlist.begin(), initlist.size())
	{
	}

	Array(std::initializer_list<RefWrapper<const T>> initlist)
		// std::initializer_list<T> doesn't live very long so we have to copy it
		: Array(tr::_consty_arena, initlist)
	{
		// using an arena for this would make the rest of the class assume you can
		// grow it later but nuh uh you can't
		_can_grow = false;
	}

	// man fuck you
	constexpr Array()
		: _can_grow(false)
	{
	}

	// Initializes the array with just an arena so you can add crap later :)
	explicit Array(Arena& arena)
		: Array(arena, 0)
	{
	}

	// mutable array to const array
	// there's no version for the other way around because, much like const_cast,
	// that'd be EVIL
	constexpr operator Array<const T>() const
	requires(!std::is_const_v<T>)
	{
		return Array<const T>(_ptr, _len);
	}

	// Similar to `operator[]`, but when getting an index out of bounds, instead
	// of panicking, it returns null, which is probably useful sometimes.
	constexpr Maybe<const T&> try_get(usize idx) const
	{
		_validate();

		if (idx >= _len) {
			return {};
		}

		// oh dear
		if constexpr (std::is_const_v<T>) {
			if constexpr (std::is_reference_v<T>) {
				return *_ptr[idx];
			}
			else {
				return _ptr[idx];
			}
		}
		else {
			if constexpr (std::is_reference_v<T>) {
				return *_arena_ptr[idx];
			}
			else {
				return _arena_ptr[idx];
			}
		}
	}

	// Similar to `operator[]`, but when getting an index out of bounds, instead
	// of panicking, it returns null, which is probably useful sometimes.
	constexpr Maybe<T&> try_get(usize idx)
	{
		_validate();

		if (idx >= _len) {
			return {};
		}

		// oh dear
		if constexpr (std::is_const_v<T>) {
			if constexpr (std::is_reference_v<T>) {
				return *_ptr[idx];
			}
			else {
				return _ptr[idx];
			}
		}
		else {
			if constexpr (std::is_reference_v<T>) {
				return *_arena_ptr[idx];
			}
			else {
				return _arena_ptr[idx];
			}
		}
	}

	constexpr const T& operator[](usize idx) const
	{
		_validate();

		Maybe<const T&> item = try_get(idx);
		if (item.is_valid()) {
			return item.unwrap();
		}
		tr::panic("index out of range: array[%zu] when the length is %zu", idx, _len);
	}

	constexpr T& operator[](usize idx)
	{
		_validate();

		Maybe<T&> item = try_get(idx);
		if (item.is_valid()) {
			return item.unwrap();
		}
		tr::panic("index out of range: array[%zu] when the length is %zu", idx, _len);
	}

	// Returns the buffer.
	constexpr RefWrapper<T>* buf() const
	{
		_validate();

		if constexpr (std::is_const_v<T>) {
			return _ptr;
		}
		else {
			return _arena_ptr;
		}
	}
	// Returns the length of the array.
	constexpr usize len() const
	{
		_validate();
		return _len;
	}
	// Returns how many items the array can hold before having to resize.
	constexpr usize cap() const
	{
		_validate();
		return _cap;
	}
	// Shorthand for `.buf()`
	constexpr RefWrapper<T>* operator*() const
	{
		return buf();
	}

	// fucking iterator
	class Iterator
	{
	public:
		// :(
		using Type =
			std::conditional_t<std::is_const_v<T>, const RefWrapper<T>, RefWrapper<T>>;

		constexpr Iterator(Type* ptr, usize idx)
			: _idx(idx)
			, _ptr(ptr)
		{
		}
		constexpr ArrayItem<T&> operator*() const
		{
			if constexpr (std::is_reference_v<T>) {
				return {_idx, **_ptr};
			}
			else {
				return {_idx, *_ptr};
			}
		}
		constexpr Iterator& operator++()
		{
			_ptr++;
			_idx++;
			return *this;
		}
		constexpr bool operator!=(const Iterator& other) const
		{
			return _ptr != other._ptr;
		}

	private:
		usize _idx;
		Type* _ptr;
	};

	constexpr Iterator begin() const
	{
		_validate();
		return Iterator(buf(), 0);
	}
	constexpr Iterator end() const
	{
		_validate();
		return Iterator(buf() + len(), len());
	}

	// Adds a new item to the array, and resizes it if necessary. This only works
	// on arena-allocated arrays, if you try to use this on an array without an
	// arena, it will panic.
	void add(T val)
	requires(!std::is_const_v<T>)
	{
		_validate();
		if (!_can_grow || _src_arena == nullptr) [[unlikely]] {
			tr::panic("array can't grow (likely not allocated from arena)");
		}

		// does it already fit?
		if (_len < _cap) [[likely]] {
			if constexpr (std::is_reference_v<T>) {
				_arena_ptr[_len++] = &val;
			}
			else {
				new (&_arena_ptr[_len++]) T(val);
			}
			return;
		}

		// reallocate array
		MutT* old_buffer = _arena_ptr;
		_cap *= 2;
		_arena_ptr = _src_arena->alloc<MutT*>(_cap * sizeof(T));

		// you may initialize with a length of 0 so you can then add crap later
		if (_len > 0) {
			tr::_copy_items<MutT>(_arena_ptr, old_buffer, _len);
		}

		if constexpr (std::is_reference_v<T>) {
			_arena_ptr[_len++] = &val;
		}
		else {
			new (&_arena_ptr[_len++]) T(val);
		}
	}

	// A bit like add() except it doesn't add items, it reserves space for adding items later.
	// This is useful if you're gonna add a gazillion items because then it will be only 1
	// memory allocation
	void reserve(usize items)
	requires(!std::is_const_v<T>)
	{
		_validate();
		if (!_can_grow || _src_arena == nullptr) [[unlikely]] {
			tr::panic("array can't grow (likely not allocated from arena)");
		}
		// just in case the math blows up or some shit
		if (items == 0) [[unlikely]] {
			return;
		}

		// does it already fit?
		if (_len + items < _cap) {
			return;
		}

		// reallocate array
		MutT* old_buffer = _arena_ptr;
		_cap = tr::max(_cap * 2, _cap + items);
		_arena_ptr = _src_arena->alloc<MutT*>(_cap * sizeof(T));

		// you may initialize with a length of 0 so you can then add crap later
		if (_len > 0) {
			tr::_copy_items<MutT>(_arena_ptr, old_buffer, _len);
		}

		// initialize the new items so nothing evil happens
		for (usize i = _len; i < _len + items; i++) {
			if constexpr (std::is_reference_v<T>) {
				_arena_ptr[i] = nullptr;
			}
			else {
				_arena_ptr[i] = T{};
			}
		}
	}

	// As the name implies, it copies the array and its items to somewhere else.
	[[nodiscard]]
	Array<T> duplicate(Arena& arena) const
	{
		_validate();
		return Array<T>{arena, buf(), len()};
	}

	// Clears the array duh. `ArrayClearBehavior::RESET_ALL_ITEMS` goes through every
	// item and calls the default constructor, which you may not want if you just want
	// to reuse the buffer for hyper-ultra-blazingly-fast-optimization
	void clear(ArrayClearBehavior behavior = ArrayClearBehavior::RESET_ALL_ITEMS)
	requires(!std::is_const_v<T>)
	{
		_validate();
		if (behavior == ArrayClearBehavior::RESET_ALL_ITEMS) {
			if constexpr (std::is_reference_v<T>) {
				for (usize i = 0; i < len(); i++) {
					_arena_ptr[i] = nullptr;
				}
			}
			else {
				for (auto [_, item] : *this) {
					item = T{};
				}
			}
		}
		_len = 0;
	}
};

// Like a C stack array, but with bounds checking and stuff so that you can sleep at night. Converts
// to an `Array<T>` automagically and is mostly constexpr.
template<usize N, typename T>
requires(!std::is_reference_v<T>)
class List
{
	T _array[N] = {};

public:
	using Type = T;

	constexpr List() {}
	constexpr List(std::initializer_list<const T> initlist)
	{
		tr::_copy_items<T>(_array, initlist.begin(), tr::min(initlist.size(), N));
	}

	constexpr Maybe<const T&> try_get(usize idx) const TR_LIFETIMEBOUND
	{
		if (idx >= N) [[unlikely]] {
			return {};
		}
		return _array[idx];
	}

	constexpr const T& operator[](usize idx) const TR_LIFETIMEBOUND
	{
		Maybe<const T&> item = try_get(idx);
		if (item.is_valid()) {
			return item.unwrap();
		}
		tr::panic("index out of range: list[%zu] when the length is %zu", idx, N);
	}

	constexpr Maybe<T&> try_get(usize idx) TR_LIFETIMEBOUND
	{
		if (idx >= N) [[unlikely]] {
			return {};
		}
		return _array[idx];
	}

	constexpr T& operator[](usize idx) TR_LIFETIMEBOUND
	{
		// exploiting constexpr to get compile-time errors
		if (std::is_constant_evaluated()) {
			throw "skill";
			if (idx >= N) {
				throw "index out of range";
			}
			return _array[idx];
		}
		else {
			Maybe<T&> item = try_get(idx);
			if (item.is_valid()) {
				return item.unwrap();
			}
			tr::panic("index out of range: list[%zu] when the length is %zu", idx, N);
		}
	}

	constexpr const T* buf() const TR_LIFETIMEBOUND
	{
		return _array;
	}

	constexpr T* buf() TR_LIFETIMEBOUND
	{
		return _array;
	}

	constexpr const T* operator*() const TR_LIFETIMEBOUND
	{
		return _array;
	}

	constexpr T* operator*() TR_LIFETIMEBOUND
	{
		return _array;
	}

	constexpr usize len() const
	{
		return N;
	}

	constexpr operator Array<const T>() const TR_LIFETIMEBOUND
	{
		return {_array, N};
	}

	constexpr operator Array<T>() TR_LIFETIMEBOUND
	{
		return {_array, N};
	}

	Array<T> duplicate(Arena& arena) const
	{
		return static_cast<Array<T>>(*this).duplicate(arena); // stupid idc
	}

	// fucking iterator
	class Iterator
	{
	public:
		constexpr Iterator(T* pointer, usize index)
			: idx(index)
			, ptr(pointer)
		{
		}
		constexpr ArrayItem<T> operator*() const
		{
			return {this->idx, *this->ptr};
		}
		constexpr Iterator& operator++()
		{
			this->ptr++;
			this->idx++;
			return *this;
		}
		constexpr bool operator!=(const Iterator& other) const
		{
			return ptr != other.ptr;
		}

	private:
		usize idx;
		Type* ptr;
	};

	constexpr Iterator begin() const
	{
		return Iterator{this->buf(), 0};
	}
	constexpr Iterator end() const
	{
		return Iterator(this->buf() + this->len(), this->len());
	}
};

} // namespace tr

#endif
