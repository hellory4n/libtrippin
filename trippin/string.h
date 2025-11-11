/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/string.h
 * Strings :)
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

#ifndef _TRIPPIN_STRING_H
#define _TRIPPIN_STRING_H

#include <cstdarg>
#include <type_traits>

#include "trippin/common.h"
#include "trippin/log.h"
#include "trippin/memory.h"

namespace tr {

// replacement for libc's `str*` functions, which are unsafe and evil. `tr::strlib` also supports
// multiple character types. should probably not be used directly (use
// `tr::String`/`tr::StringBuilder` etc)
namespace strlib {
	// why won't you let me use strlen in constexpr :(
	template<Character T>
	constexpr usize constexpr_strlen(const T* str)
	{
		if (str == nullptr) {
			return 0;
		}

		usize i = 0;
		while (str[i] != '\0') {
			i++;
		}
		return i;
	}

	// source: https://en.cppreference.com/w/c/string/byte/memset#Notes
	// > memset may be optimized away (under the as-if rules) if the object modified by this
	// > function is not accessed again for the rest of its lifetime (e.g., gcc bug 8537). For
	// > that reason, this function cannot be used to scrub memory (e.g., to fill an array that
	// > stored a password with zeroes).
	void explicit_memset(void* ptr, usize len, byte val);

	// windows uses a different function for this for reasons unbeknownst to man. this copies
	// the va_list for you so no need to do that yourself.
	usize sprintf_len(const char* fmt, va_list arg);

	// utf-8 lib (just an utf8proc wrapper)

	// converts an unicode string to uppercase.
	void utf8_to_uppercase(const char* s, usize len, char* out);
	// converts an unicode string to lowercase.
	void utf8_to_lowercase(const char* s, usize len, char* out);
}

// more utf-8 stuff

// Returns true if a codepoint is valid, regardless of whether or not it has been assigned a
// value by the current Unicode standard.
bool is_unicode_codepoint_valid(char32 c);

class StringBuilder;

// A view into immutable UTF-8 strings. Strings are just a pointer + length, with the underlying
// data being const. If you want to modify it, copy the data, or use `StringBuilder`. The 'default'
// string type, equivalent to `std::string_view`. Always null-terminated, so it can be safely used
// with C libraries.
class String
{
	const char* _ptr;
	usize _len;

	void _validate() const
	{
		if (_ptr == nullptr) [[unlikely]] {
			tr::panic(
				"string cannot be nullptr, use tr::Maybe<tr::String> if this is "
				"intentional"
			);
		}
	}

public:
	using Type = char;

	constexpr String(const char* str, usize len)
		: _ptr(str)
		, _len(len + 1)
	{
		// does len already include the null terminator?
		if (len > 0) {
			if (_ptr[len - 1] == '\0') {
				_len--;
			}
		}
	}

	constexpr String(const char* str)
		: String(str, tr::strlib::constexpr_strlen(str))
	{
	}

	constexpr String()
		: _ptr("")
		, _len(0)
	{
	}

	String(char c)
		// c++ can't do (char[]){c, '\0'} like c99 can :)))))
		: String(Array<const char>{c, '\0'}.buf())
	{
	}

	// Copies an existing string ptr to an arena
	String(Arena& arena, const char* str, usize len)
		: _len(len + 1)
	{
		// does len already include the null terminator?
		if (len > 0) {
			if (str[len - 1] == '\0') {
				_len--;
			}
		}

		char* newptr = arena.alloc<char*>(len * sizeof(char));
		memcpy(newptr, str, len * sizeof(char));
		_ptr = newptr;
	}

	// Copies an existing string ptr to an arena
	String(Arena& arena, const char* str)
		: String(arena, str, tr::strlib::constexpr_strlen(str))
	{
	}

	// Creates a `String` from a `StringBuilder`
	explicit String(StringBuilder sb);

	// Creates a `String` by copying a `StringBuilder`
	String(Arena& arena, StringBuilder sb);

	// since the C++ committee hates its users, we must explicitly cast from char8 to char
	// technically undefined behavior but should work anywhere that uses ASCII/UTF-8 (so most
	// places)
	String(const char8* str, usize len)
		: String(reinterpret_cast<const char*>(str), len)
	{
	}
	String(const char8* str)
		: String(reinterpret_cast<const char*>(str), tr::strlib::constexpr_strlen(str))
	{
	}
	String(Arena& arena, const char8* str)
		: String(arena, reinterpret_cast<const char*>(str),
			 tr::strlib::constexpr_strlen(str))
	{
	}

	// Does NOT include the null terminator. In bytes, use `codepoint_len()` if you need the
	// amount of codepoints
	constexpr usize len() const
	{
		return _len - 1;
	}

	// Returns the amount of codepoints, not to be confused with `len()` which returns the
	// length in bytes.
	usize codepoint_len() const;

	constexpr const char* buf() const
	{
		if (!std::is_constant_evaluated()) {
			_validate();
		}
		return _ptr;
	}

	constexpr const char* operator*() const
	{
		if (!std::is_constant_evaluated()) {
			_validate();
		}
		return _ptr;
	}

	// Similar to `operator[]`, but when getting an index out of bounds, instead
	// of panicking, it returns null, which is probably useful sometimes. Note this works with
	// bytes, NOT codepoints. Use `try_get_codepoint()` if you need codepoints.
	constexpr Maybe<char> try_get(usize idx) const
	{
		if (!std::is_constant_evaluated()) {
			_validate();
		}
		if (idx >= this->_len) {
			return {};
		}
		return this->_ptr[idx];
	}

	// Note this works with bytes, NOT codepoints. Use `get_codepoint()` if you need codepoints.
	constexpr char operator[](usize idx) const
	{
		// """"""""""constexpr"""""""""" function
		if (!std::is_constant_evaluated()) {
			_validate();
		}
		Maybe<char> item = try_get(idx);
		if (item.is_valid()) {
			return item.unwrap();
		}
		tr::panic("index out of range: string[%zu] when the length is %zu", idx, _len);
	}

	Maybe<char32> try_get_codepoint(usize idx) const;
	char32 get_codepoint(usize idx) const;

	class Iterator
	{
	public:
		Iterator(const char* ptr, usize idx, usize len)
			: _ptr(ptr)
			, _idx(idx)
			, _len(len)
		{
		}
		ArrayItem<char32> operator*() const;
		Iterator& operator++();
		constexpr bool operator!=(const Iterator& other) const
		{
			return _ptr != other._ptr;
		}

	private:
		const char* _ptr;
		usize _idx;
		usize _len;
	};

	// Works with codepoints, just iterate the buffer manually if you need bytes
	Iterator begin() const
	{
		return Iterator{buf(), 0, len()};
	}
	Iterator end() const
	{
		return Iterator{buf() + len(), len(), len()};
	}

	// As the name implies, it copies the string and its items to somewhere else.
	[[nodiscard]]
	String duplicate(Arena& arena) const
	{
		return {arena, buf(), len()};
	}

	bool operator==(String other) const;

	bool operator!=(String other) const
	{
		return !(*this == other);
	}

	bool operator==(const char* other) const
	{
		return *this == String{other};
	}

	bool operator!=(const char* other) const
	{
		return *this != String{other};
	}

	bool operator==(const char8* other) const
	{
		return *this == String{other};
	}

	bool operator!=(const char8* other) const
	{
		return *this != String{other};
	}

	// Converts the UTF-8 data to UTF-32, which might be useful sometimes
	Array<char32> to_utf32(Arena& arena) const;

	// Gets a substring. The returned string includes the end character. Note that `start` and
	// `end` are NOT in codepoints but instead in indexes (bytes for UTF-8, 2 bytes for UTF-16,
	// 4 bytes for UTF-32)
	[[nodiscard]]
	String substr(Arena& arena, usize start, usize end) const;

	// Returns an array with all of the indexes containing that character (the index is where it
	// starts). Note that the indexes and `start` and `end` are NOT in codepoints.
	Array<usize> find(Arena& arena, char c, usize start = 0, usize end = 0) const;

	// TODO find_codepoint? idk how useful that'd be

	// Returns an array with all of the indexes containing the substring (the index is where it
	// starts). Note that the indexes and `start` and `end` are NOT in codepoints.
	Array<usize> find(Arena& arena, String str, usize start = 0, usize end = 0) const;

	// It concatenates 2 strings lmao. Usually you should use `tr::fmt` or `tr::StringBuilder`.
	[[nodiscard]]
	String concat(Arena& arena, String other) const;

	// If true, the string starts with that other crap.
	bool starts_with(String str) const;

	// If true, the string ends with that other crap.
	bool ends_with(String str) const;

	// Gets the filename in a path, e.g. returns `file.txt` for `/path/to/file.txt`
	[[nodiscard]]
	String file(Arena& arena) const;

	// Gets the directory in a path e.g. returns `/path/to` for `/path/to/file.txt`
	[[nodiscard]]
	String directory(Arena& arena) const;

	// Returns the extension in a path, e.g. returns `.txt` for `/path/to/file.txt`, `.blend.1`
	// for `teapot.blend.1`, and an empty string for `.gitignore`
	[[nodiscard]]
	String extension(Arena& arena) const;

	// If true, the path is absolute. Else, it's relative.
	bool is_absolute() const;

	// If true, the path is relative. Else, it's absolute.
	bool is_relative() const
	{
		return !this->is_absolute();
	}

	// Replaces a character with another character.
	[[nodiscard]]
	String replace(Arena& arena, char from, char to) const;

	// TODO replace_codepoint?

	// Splits the string into several substrings using the specified delimiter.
	[[nodiscard]]
	Array<String> split(Arena& arena, char delimiter) const;

	// TODO split_by_codepoint?
};

// Mutable string. You can change it and stuff. 90% of the time you should use `tr::String`
// instead. Always null-terminated, so it can be safely used with C libraries.
class StringBuilder
{
	Array<char> _array;

public:
	// Initializes an empty string builder at an arena.
	StringBuilder(Arena& arena, usize len)
		: _array(arena, len + 1)
	{
	}

	// Initializes a string builder from a buffer. (the data is copied into the arena)
	StringBuilder(Arena& arena, const char* str, usize len)
	{
		// does len already include the null terminator?
		if (len > 0) {
			if (str[len - 1] == '\0') {
				len--;
			}
		}
		_array = {arena, str, len + 1};
	}

	// Initializes a string builder from a buffer. (the data is copied into the arena)
	StringBuilder(Arena& arena, const char* str)
		: _array(arena, str, tr::strlib::constexpr_strlen(str) + 1)
	{
	}

	// man fuck you
	StringBuilder()
		: _array()
	{
	}

	// Initializes the string with just an arena so you can add crap later :)
	StringBuilder(Arena& arena)
		: _array(arena)
	{
	}

	// Initializes a string builder by copying a regular old boring string
	StringBuilder(Arena& arena, String str)
		: StringBuilder(arena, str.buf(), str.len())
	{
	}

	// string builder to string view
	constexpr operator String() const
	{
		return String(*_array, _array.len());
	}

	// Similar to `operator[]`, but when getting an index out of bounds, instead
	// of panicking, it returns null, which is probably useful sometimes. Note this works with
	// bytes, NOT codepoints.
	constexpr Maybe<char&> try_get(usize idx)
	{
		return _array.try_get(idx);
	}

	// Similar to `operator[]`, but when getting an index out of bounds, instead
	// of panicking, it returns null, which is probably useful sometimes. Note this works with
	// bytes, NOT codepoints.
	constexpr Maybe<const char&> try_get(usize idx) const
	{
		return _array.try_get(idx);
	}

	// Note this works with bytes, NOT codepoints.
	constexpr const char& operator[](usize idx) const
	{
		return _array[idx];
	}

	// Note this works with bytes, NOT codepoints.
	constexpr char& operator[](usize idx)
	{
		return _array[idx];
	}

	// Returns the buffer. Bufma blals.
	constexpr char* buf() const
	{
		return _array.buf();
	}

	// Returns the length of the string.
	constexpr usize len() const
	{
		return _array.len() - 1;
	}

	// Returns how many characters the string can hold before having to resize.
	constexpr usize cap() const
	{
		return _array.cap();
	}

	// Shorthand for `.buf()`
	constexpr char* operator*() const
	{
		return _array.buf();
	}

	class Iterator
	{
	public:
		Iterator(char* ptr, usize idx, usize len)
			: _ptr(ptr)
			, _idx(idx)
			, _len(len)
		{
		}
		ArrayItem<char32> operator*() const;
		Iterator& operator++();
		constexpr bool operator!=(const Iterator& other) const
		{
			return _ptr != other._ptr;
		}

	private:
		char* _ptr;
		usize _idx;
		usize _len;
	};

	// Works with codepoints, just iterate the buffer manually if you need bytes
	Iterator begin() const
	{
		return Iterator{buf(), 0, len()};
	}
	Iterator end() const
	{
		return Iterator{buf() + len(), len(), len()};
	}

	// As the name implies, it copies the string builder and its items to somewhere else.
	[[nodiscard]]
	StringBuilder duplicate(Arena& arena) const
	{
		return {arena, buf(), len()};
	}

	// Clears the string builder duh. `ArrayClearBehavior::RESET_ALL_ITEMS` sets the entire
	// buffer to 0, which you may not want if you just want to reuse the buffer for
	// hyper-ultra-blazingly-fast-optimization
	void clear(ArrayClearBehavior behavior = ArrayClearBehavior::RESET_ALL_ITEMS)
	{
		_array.clear(behavior);
	}

	bool operator==(String other) const;

	bool operator!=(String other) const
	{
		return !(*this == other);
	}

	bool operator==(const char* other) const
	{
		return *this == String{other};
	}

	bool operator!=(const char* other) const
	{
		return *this != String{other};
	}

	// Adds a character to the string.
	void append(char c);

	// Appends another string to the string. Incredible indeed.
	void append(String s);

	// Appends a formatted string with formatting because that's what formatted means.
	_TR_PRINTF_ATTR(2, 3)
	void appendf(const char* fmt, ...);
};

inline String::String(StringBuilder sb)
	: _ptr(*sb)
	, _len(sb.len() + 1)
{
}

inline String::String(Arena& arena, StringBuilder sb)
	: String(arena, *sb, sb.len())
{
}

String fmt_args(Arena& arena, const char* fmt, va_list arg);

// It's just `sprintf` for `tr::String` lmao.
[[gnu::format(printf, 2, 3)]]
String fmt(Arena& arena, const char* fmt, ...);

// Uses fancy allocation technology to return a temporary string from printf-like formatting with
// near zero overhead. However, the strings are very much temporary, so they should be used as soon
// as possible, otherwise the strings will get poisoned (using the constant 0xbebebebe...), and then
// eventually overwritten as the program runs. If you need a string that lives longer, duplicate the
// string, or use the regular `tr::fmt`.
[[gnu::format(printf, 1, 2)]]
String tmp_fmt(const char* fmt, ...);

}

#endif
