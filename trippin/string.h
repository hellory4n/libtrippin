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

template<typename T>
concept Character =
	std::is_same_v<T, char> || std::is_same_v<T, wchar_t> || std::is_same_v<T, char8> ||
	std::is_same_v<T, char16> || std::is_same_v<T, char32>;

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

	// in true C fashion, these are all (probably) unsafe
	// these all work in bytes, if the character size matters then you should pass that through
	// `ch_len`. note 'character' and 'byte' are different terms used here, a character may or
	// may not be 1 byte. all input lengths are expected to *not* include the null terminator.

	// source: https://en.cppreference.com/w/c/string/byte/memset#Notes
	// > memset may be optimized away (under the as-if rules) if the object modified by this
	// > function is not accessed again for the rest of its lifetime (e.g., gcc bug 8537). For
	// > that reason, this function cannot be used to scrub memory (e.g., to fill an array that
	// > stored a password with zeroes).
	void explicit_memset(void* ptr, usize len, byte val);
	// returns true if A and B are exactly equal
	bool strs_equal(const byte* a, usize a_len, const byte* b, usize b_len);
	// copies a section of S into out, out buffer must have +1 character for the null terminator
	void substr(const byte* s, usize len, usize start, usize end, byte* out, usize ch_len);
	// returns an array of indexes (bytes) allocated from arena, referring to the parts of the
	// string with the specified character (can be multiple bytes)
	Array<usize> find_char(
		Arena& arena, const byte* s, usize len, usize start, usize end, const byte* ch,
		usize ch_len
	);
	// returns an array of indexes (bytes) allocated from arena, referring to the parts of the
	// strings with the specified string
	Array<usize> find_str(
		Arena& arena, const byte* s, usize len, usize start, usize end, const byte* substr,
		usize substr_len
	);
	// concatenates 2 strings into out. out buffer's size should be a_len + b_len + 1 character
	// for the null terminator.
	void
	concat(const byte* a, usize a_len, const byte* b, usize b_len, byte* out, usize ch_len);
	// returns true if the first [substr_len] bytes of `s` are equal to `substr`
	bool starts_with(const byte* s, usize s_len, const byte* substr, usize substr_len);
	// returns true if the last [substr_len] bytes of `s` are equal to `substr`
	bool ends_with(const byte* s, usize s_len, const byte* substr, usize substr_len);
	// replaces a character in the string with another string
	void
	replace(const byte* s, usize s_len, const byte* from_ch, const byte* to_ch, usize ch_len,
		byte* out);
	// im splitting by char<3
	Array<byte*>
	split_by_char(Arena& arena, const byte* s, usize s_len, const byte* ch, usize ch_len);

	// these are all utf-8 only (as it would be too annoying to make them encoding-independent),
	// for all other encodings you must first convert to utf-8, call the function, then convert
	// back to the desired encoding.

	// in a path, gets the file in the path. writes the ptr into `out` and the length into
	// `out_len`
	void strfile(Arena& arena, const char8* s, usize len, char8** out, usize* out_len);
	// in a path, gets the directory in the path. writes the ptr into `out` and the length into
	// `out_len`
	void strdir(Arena& arena, const char8* s, usize len, char8** out, usize* out_len);
	// in a path, gets the file extension in the path. writes the ptr into `out` and the length
	// into `out_len`. `out` is nullptr if there is no extension.
	void strext(Arena& arena, const char8* s, usize len, char8** out, usize* out_len);
	// returns true if the string is an absolute path
	bool strabsolute(const char8* s, usize len);
}

template<Character T>
requires(!std::is_const_v<T>)
class BaseStringBuilder;

// A view into immutable strings. Strings are just a pointer + length, with the underlying data
// being const. If you want to modify it, copy the data, or use `StringBuilder`. The 'default'
// string type, equivalent to `std::string_view`.
template<Character T>
class BaseString
{
	const T* _ptr;
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
	using Type = T;

	constexpr BaseString(const T* str, usize len)
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

	constexpr BaseString(const T* str)
		: BaseString(str, tr::strlib::constexpr_strlen(str))
	{
	}

	constexpr BaseString()
		: _ptr("")
		, _len(0)
	{
	}

	BaseString(T c)
		// c++ can't do (char[]){c, '\0'} like c99 can :)))))
		: BaseString(Array<const T>{c, '\0'}.buf())
	{
	}

	// Copies an existing string ptr to an arena
	BaseString(Arena& arena, const T* str, usize len)
		: _len(len + 1)
	{
		// does len already include the null terminator?
		if (len > 0) {
			if (str[len - 1] == '\0') {
				_len--;
			}
		}

		T* newptr = static_cast<T*>(arena.alloc(len * sizeof(T)));
		memcpy(newptr, str, len * sizeof(T));
		_ptr = newptr;
	}

	// Copies an existing string ptr to an arena
	BaseString(Arena& arena, const T* str)
		: BaseString(arena, str, tr::strlib::constexpr_strlen(str))
	{
	}

	// Creates a `String` from a `StringBuilder`
	explicit BaseString(BaseStringBuilder<T> sb);

	// Creates a `String` by copying a `StringBuilder`
	BaseString(Arena& arena, BaseStringBuilder<T> sb);

	// Does NOT include the null terminator
	constexpr usize len() const
	{
		return _len - 1;
	}

	const T* buf() const
	{
		_validate();
		return _ptr;
	}

	const T* operator*() const
	{
		_validate();
		return _ptr;
	}

	// Similar to `operator[]`, but when getting an index out of bounds, instead
	// of panicking, it returns null, which is probably useful sometimes.
	Maybe<T> try_get(usize idx) const
	{
		_validate();
		if (idx >= this->_len) {
			return {};
		}
		return this->_ptr[idx];
	}

	T operator[](usize idx) const
	{
		_validate();
		Maybe<T> item = try_get(idx);
		if (!item.is_valid()) {
			tr::panic(
				"index out of range: string[%zu] when the length is %zu", idx, _len
			);
		}
		return item.unwrap();
	}

	class Iterator
	{
	public:
		constexpr Iterator(const T* pointer, usize index)
			: _idx(index)
			, _ptr(pointer)
		{
		}
		constexpr ArrayItem<T> operator*() const
		{
			return {_idx, *_ptr};
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
		return Iterator(buf(), 0);
	}
	constexpr Iterator end() const
	{
		return Iterator(buf() + len() - 1, len());
	}

	// As the name implies, it copies the array and its items to somewhere else.
	[[nodiscard]]
	BaseString duplicate(Arena& arena) const
	{
		return {arena, buf(), len()};
	}

	bool operator==(BaseString other) const
	{
		return tr::strlib::strs_equal(
			reinterpret_cast<const byte*>(buf()), len() * sizeof(T),
			reinterpret_cast<const byte*>(other.buf()), other.len() * sizeof(T)
		);
	}

	bool operator!=(BaseString other) const
	{
		return !(*this == other);
	}

	bool operator==(const T* other) const
	{
		return *this == BaseString{other};
	}

	bool operator!=(const T* other) const
	{
		return *this != BaseString{other};
	}

	// Gets a substring. The returned string includes the end character.
	[[nodiscard]]
	BaseString substr(Arena& arena, usize start, usize end) const
	{
		T* buffer = static_cast<T*>(arena.alloc((end - start + 1) * sizeof(T)));
		tr::strlib::substr(
			reinterpret_cast<const byte*>(buf()), len() * sizeof(T), start * sizeof(T),
			end * sizeof(T), reinterpret_cast<byte*>(buffer), sizeof(T)
		);
		return {buffer, end - start + 1};
	}

	// Returns an array with all of the indexes containing that character (the index is where it
	// starts)
	Array<usize> find(Arena& arena, T c, usize start = 0, usize end = 0) const
	{
		if (end == 0 || end > len()) {
			end = len();
		}
		Array<usize> indexes = tr::strlib::find_char(
			arena, buf(), len() * sizeof(T), start * sizeof(T), end * sizeof(T), &c,
			sizeof(T)
		);

		// strlib::find_char returns indexes in bytes, a character may be more than that
		// the character may be so very special...
		if constexpr (sizeof(T) > 1) {
			for (auto [_, idx] : indexes) {
				idx = idx / sizeof(T);
			}
		}
		return indexes;
	}

	// Returns an array with all of the indexes containing the substring (the index is where it
	// starts)
	Array<usize> find(Arena& arena, BaseString str, usize start = 0, usize end = 0) const
	{
		if (end == 0 || end > len()) {
			end = len();
		}
		Array<usize> indexes = tr::strlib::find_str(
			arena, reinterpret_cast<const byte*>(buf()), len() * sizeof(T),
			start * sizeof(T), end * sizeof(T),
			reinterpret_cast<const byte*>(str.buf()), str.len()
		);

		// strlib::find_str returns indexes in bytes, a character may be more than that
		// the character may be so very special...
		if constexpr (sizeof(T) > 1) {
			for (auto [_, idx] : indexes) {
				idx = idx / sizeof(T);
			}
		}
		return indexes;
	}

	// It concatenates 2 strings lmao.
	[[nodiscard]]
	BaseString concat(Arena& arena, BaseString other) const
	{
		T* newstr = static_cast<T*>(arena.alloc((len() + 1) * sizeof(T)));
		tr::strlib::concat(
			reinterpret_cast<const byte*>(buf()), len() * sizeof(T),
			reinterpret_cast<const byte*>(other.buf()), other.len() * sizeof(T),
			reinterpret_cast<byte*>(newstr), sizeof(T)
		);
		return newstr;
	}

	// If true, the string starts with that other crap.
	bool starts_with(BaseString str) const
	{
		return tr::strlib::starts_with(
			reinterpret_cast<const byte*>(buf()), len() * sizeof(T),
			reinterpret_cast<const byte*>(str.buf()), str.len() * sizeof(T)
		);
	}

	// If true, the string ends with that other crap.
	bool ends_with(BaseString str) const
	{
		return tr::strlib::ends_with(
			reinterpret_cast<const byte*>(buf()), len() * sizeof(T),
			reinterpret_cast<const byte*>(str.buf()), str.len() * sizeof(T)
		);
	}

	// Gets the filename in a path, e.g. returns `file.txt` for `/path/to/file.txt`
	[[nodiscard]]
	BaseString file(Arena& arena) const
	{
		// FIXME this wont work on other encodings
		char8* newstr;
		usize newlen;
		tr::strlib::strfile(
			arena, reinterpret_cast<const char8*>(buf()), len() * sizeof(T), &newstr,
			&newlen
		);
		return {reinterpret_cast<const T*>(newstr), newlen};
	}

	// Gets the directory in a path e.g. returns `/path/to` for `/path/to/file.txt`
	[[nodiscard]]
	BaseString directory(Arena& arena) const
	{
		char8* newstr;
		usize newlen;
		tr::strlib::strdir(
			arena, reinterpret_cast<const char8*>(buf()), len() * sizeof(T), &newstr,
			&newlen
		);
		return {reinterpret_cast<const T*>(newstr), newlen};
	}

	// Returns the extension in a path, e.g. returns `.txt` for `/path/to/file.txt`, `.blend.1`
	// for `teapot.blend.1`, and an empty string for `.gitignore`
	[[nodiscard]]
	BaseString extension(Arena& arena) const
	{
		char8* newstr;
		usize newlen;
		tr::strlib::strext(
			arena, reinterpret_cast<const char8*>(buf()), len() * sizeof(T), &newstr,
			&newlen
		);
		return {reinterpret_cast<const T*>(newstr), newlen};
	}

	// If true, the path is absolute. Else, it's relative.
	bool is_absolute() const
	{
		return tr::strlib::strabsolute(
			reinterpret_cast<const char8*>(buf()), len() * sizeof(T)
		);
	}

	// If true, the path is relative. Else, it's absolute.
	bool is_relative() const
	{
		return !this->is_absolute();
	}

	// Replaces a character with another character.
	[[nodiscard]]
	BaseString replace(Arena& arena, T from, T to) const
	{
		T* newstr = arena.alloc((len() + 1) * sizeof(T));
		tr::strlib::replace(buf(), len() * sizeof(T), &from, &to, sizeof(T), newstr);
		return {newstr, len()};
	}

	// Splits the string into several substrings using the specified delimiter.
	[[nodiscard]]
	Array<BaseString> split(Arena& arena, T delimiter) const
	{
		// FIXME tr::strlib::split_by_char might be dogshit
		Array<byte*> mn = tr::strlib::split_by_char(
			arena, reinterpret_cast<const byte*>(buf()), len() * sizeof(T),
			reinterpret_cast<const byte*>(&delimiter), sizeof(T)
		);

		// type fucking to get it to not be a byte ptr
		Array<BaseString> thej{arena, mn.len()};
		for (auto [i, ptr] : mn) {
			thej[i] = reinterpret_cast<const T*>(ptr);
		}
		return thej;
	}
};

// A view into immutable UTF-8 strings.
using String8 = BaseString<char>;
// A view into immutable UTF-8 strings.
using String = String8; // utf-8 is the default

// TODO encoding conversion functions:
// utf-8 to utf-16
// utf-8 to utf-32
// utf-16 to utf-8
// utf-16 to utf-32
// utf-32 to utf-8
// utf-32 to utf-16

// Mutable string. You can change it and stuff. 90% of the time you should use `tr::String` instead.
template<Character T>
requires(!std::is_const_v<T>)
class BaseStringBuilder
{
	Array<T> _array;

public:
	// Initializes an empty string builder at an arena.
	BaseStringBuilder(Arena& arena, usize len)
		: _array(arena, len + 1)
	{
	}

	// Initializes a string builder from a buffer. (the data is copied into the arena)
	BaseStringBuilder(Arena& arena, const T* str, usize len)
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
	BaseStringBuilder(Arena& arena, const T* str)
		: _array(arena, str, tr::strlib::constexpr_strlen(str) + 1)
	{
	}

	// man fuck you
	BaseStringBuilder()
		: _array()
	{
	}

	// Initializes the string with just an arena so you can add crap later :)
	BaseStringBuilder(Arena& arena)
		: _array(arena)
	{
	}

	// Initializes a string builder by copying a regular old boring string
	BaseStringBuilder(Arena& arena, BaseString<T> str)
		: BaseStringBuilder(arena, str.buf(), str.len())
	{
	}

	// string builder to string view
	operator BaseString<T>() const
	{
		return BaseString<T>(*_array, _array.len());
	}

	// Similar to `operator[]`, but when getting an index out of bounds, instead
	// of panicking, it returns null, which is probably useful sometimes.
	Maybe<T&> try_get(usize idx)
	{
		return _array.try_get(idx);
	}

	// Similar to `operator[]`, but when getting an index out of bounds, instead
	// of panicking, it returns null, which is probably useful sometimes.
	Maybe<const T&> try_get(usize idx) const
	{
		return _array.try_get(idx);
	}

	const T& operator[](usize idx) const
	{
		return _array[idx];
	}

	T& operator[](usize idx)
	{
		return _array[idx];
	}

	// Returns the buffer. Bufma blals.
	constexpr T* buf() const
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
	constexpr T* operator*() const
	{
		return _array.buf();
	}

	constexpr Array<T>::Iterator begin() const
	{
		return _array.begin();
	}
	constexpr Array<T>::Iterator end() const
	{
		// not going directly thru _array.end() so that it doesn't iterate over the null
		// terminator
		return Array<T>::Iterator(buf() + len() - 1, len());
	}

	// As the name implies, it copies the array and its items to somewhere else.
	[[nodiscard]]
	BaseStringBuilder duplicate(Arena& arena) const
	{
		return {arena, buf(), len()};
	}

	// Clears the array duh. `ArrayClearBehavior::RESET_ALL_ITEMS` goes through every
	// item and calls the default constructor, which you may not want if you just want
	// to reuse the buffer for hyper-ultra-blazingly-fast-optimization
	void clear(ArrayClearBehavior behavior = ArrayClearBehavior::RESET_ALL_ITEMS)
	{
		_array.clear(behavior);
	}

	// Adds a character to the string.
	void append(T c)
	{
		_array.add(c);
	}
};

template<Character T>
BaseString<T>::BaseString(BaseStringBuilder<T> sb)
	: _ptr(*sb)
	, _len(sb.len() + 1)
{
}

// Mutable UTF-8 string. You can change it and stuff.
using StringBuilder8 = BaseStringBuilder<char>;
// Mutable UTF-8 string. You can change it and stuff.
using StringBuilder = StringBuilder8; // utf-8 is the default

String fmt_args(Arena& arena, const char* fmt, va_list arg);

// It's just `sprintf` for `tr::String` lmao.
[[gnu::format(printf, 2, 3)]]
String fmt(Arena& arena, const char* fmt, ...);

}

#endif
