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

#include <stdarg.h>

#include "trippin/memory.h"
#include "trippin/common.h"

namespace tr {

// why won't you let me use strlen in constexpr :(
constexpr usize constexpr_strlen(const char* str)
{
	usize i = 0;
	while (str[i] != '\0') i++;
	return i;
}

// Literally just a wrapper around `tr::Array`, so it works better with strings. Also all of the functions
// that return strings copy the original strings first. Strings don't own the value and don't use fancy
// RAII fuckery, so you can pass them by value.
class String
{
	Array<char> array = {};

public:
	// Initializes a string from an arena and C string.
	explicit String(Arena& arena, const char* str, usize len)
	{
		this->array = Array<char>(arena, const_cast<char*>(str), len + 1);
	}

	// Initializes an empty string from an arena
	explicit String(Arena& arena, usize len)
	{
		this->array = Array<char>(arena, len + 1);
	}

	// Initializes a string from any C string. You really should only use this for temporary arrays.
	constexpr explicit String(const char* str, usize len)
	{
		this->array = Array<char>(const_cast<char*>(str), len + 1);
	}

	// Initializes a string from any C string. You really should only use this for temporary strings.
	constexpr String(const char* str) : String(str, tr::constexpr_strlen(str)) {}

	constexpr String() : String("") {}

	String(char c)
	{
		char man[2] = {c, '\0'};
		String(man, 2);
	}

	// man
	constexpr char& operator[](usize idx) const { return this->array[idx]; }
	// Returns the length, doesn't include the null terminator
	usize len() const { return this->array.len() - 1; }
	char* buf() const { return this->array.buf(); }
	operator char*() const { return this->buf(); }
	operator const char*() const { return this->buf(); }
	Array<char>::Iterator begin() const { return this->array.begin(); }
	// this one is different since you don't want to iterate over the null terminator
	Array<char>::Iterator end() const { return Array<char>::Iterator(const_cast<char*>(this->buf()) + this->len() - 1, this->len() - 1); }
	String duplicate(Arena& arena) const
	{
		Array<char> arrayma = this->array.duplicate(arena);
		return String(arrayma.buf(), arrayma.len() + 1);
	}
	// i know .add() is missing

	// special string crap
	bool operator==(const String& other) const;
	bool operator!=(const String& other) const { return !(*this == other); }
	bool operator==(const char* other)   const { return *this == String(other); }
	bool operator!=(const char* other)   const { return *this != String(other); }

	// Gets a substring. The returned string doesn't include the end character.
	[[nodiscard]]
	String substr(Arena& arena, usize start, usize end) const;

	// Returns an array with all of the indexes containing the substring (the index is where it starts)
	Array<usize> find(Arena& arena, String str, usize start = 0, usize end = 0) const;

	// It concatenates 2 strings lmao.
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

	// Returns the extension in a path, e.g. returns `.txt` for `/path/to/file.txt`, `.blend.1` for
	// `teapot.blend.1`, and an empty string for `.gitignore`
	[[nodiscard]]
	String extension(Arena& arena) const;

	// If true, the path is absolute. Else, it's relative.
	bool is_absolute() const;

	// If true, the path is relative. Else, it's absolute.
	bool is_relative() const { return !this->is_absolute(); }

	// Replaces a character with another character.
	[[nodiscard]]
	String replace(Arena& arena, char from, char to) const;

	// Splits the string into several substrings using the specified delimiter.
	[[nodiscard]]
	Array<String> split(Arena& arena, char delimiter) const;
};

// It's just `sprintf` for `tr::String` lmao.
[[gnu::format(printf, 3, 4)]]
[[deprecated("specifying size is no longer necessary (and there's not a good reason to specify it, also the function is tr::fmt now lmao)")]]
String sprintf(Arena& arena, usize maxlen, const char* fmt, ...);

String fmt_args(Arena& arena, const char* fmt, va_list arg);

// It's just `sprintf` for `tr::String` lmao.
[[gnu::format(printf, 2, 3)]]
String fmt(Arena& arena, const char* fmt, ...);


// TODO StringBuilder

}

#endif
