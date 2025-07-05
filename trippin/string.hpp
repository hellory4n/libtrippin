/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/string.hpp
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

#include <string.h>

#include "memory.hpp"
#include "common.hpp"

namespace tr {

// Literally just a wrapper around `tr::Array`, so it works better with strings. Also all of the functions
// that return strings copy the original strings first.
class String
{
	Array<char> array;

public:
	// Initializes a string from an arena and C string.
	explicit String(Ref<Arena> arena, const char* str, usize len)
	{
		this->array = Array<char>(arena, const_cast<char*>(str), len + 1);
	}

	// Initializes an empty string from an arena
	explicit String(Ref<Arena> arena, usize len)
	{
		this->array = Array<char>(arena, len + 1);
	}

	// Initializes a string from any C string. You really should only use this for temporary arrays.
	explicit String(const char* str, usize len)
	{
		this->array = Array<char>(const_cast<char*>(str), len + 1);
	}

	// Initializes a string from any C string. You really should only use this for temporary arrays.
	String(const char* str) : String(str, strlen(str)) {}
	String() : String("") {}

	String(char c)
	{
		char man[2] = {c, '\0'};
		String(man, 2);
	}

	// man
	char& operator[](usize idx) const { return this->array[idx]; }
	// Doesn't include the null terminator
	usize length() const { return this->array.length() - 1; }
	char* buffer() const { return this->array.buffer(); }
	operator char*() const { return this->buffer(); }
	operator const char*() const { return this->buffer(); }
	Array<char>::Iterator begin() const { return this->array.begin(); }
	// this one is different since you don't want to iterate over the null terminator
	Array<char>::Iterator end() const { return Array<char>::Iterator(const_cast<char*>(this->buffer()) + this->length() - 1, this->length() - 1); }
	String duplicate(Ref<Arena> arena) const
	{
		Array<char> arrayma = this->array.duplicate(arena);
		return String(arrayma.buffer(), arrayma.length() + 1);
	}
	// i know .add() is missing

	// special string crap
	bool operator==(const String& other) const;
	bool operator!=(const String& other) const { return !(*this == other); }
	bool operator==(const char* other)   const { return *this == String(other); }
	bool operator!=(const char* other)   const { return *this != String(other); }

	// Gets a substring. The returned string doesn't include the end character.
	[[nodiscard]] String substr(Ref<Arena> arena, usize start, usize end) const;

	// Returns an array with all of the indexes containing the substring (the index is where it starts)
	Array<usize> find(Ref<Arena> arena, String str, usize start = 0, usize end = 0) const;

	// It concatenates 2 strings lmao.
	[[nodiscard]] String concat(Ref<Arena> arena, String other) const;

	// If true, the string starts with that other crap.
	bool starts_with(String str) const;

	// If true, the string ends with that other crap.
	bool ends_with(String str) const;

	// Gets the filename in a path, e.g. returns `file.txt` for `/path/to/file.txt`
	[[nodiscard]] String file(Ref<Arena> arena) const;

	// Gets the directory in a path e.g. returns `/path/to` for `/path/to/file.txt`
	[[nodiscard]] String directory(Ref<Arena> arena) const;

	// Returns the extension in a path, e.g. returns `.txt` for `/path/to/file.txt`, `.blend.1` for
	// `teapot.blend.1`, and an empty string for `.gitignore`
	[[nodiscard]] String extension(Ref<Arena> arena) const;

	// If true, the path is absolute. Else, it's relative.
	bool is_absolute() const;

	// If true, the path is relative. Else, it's absolute.
	bool is_relative() const { return !this->is_absolute(); }

	// Replaces a character with another character.
	[[nodiscard]] String replace(Ref<Arena> arena, char from, char to) const;

	// Splits the string into several substrings using the specified delimiter.
	[[nodiscard]] Array<String> split(Ref<Arena> arena, char delimiter) const;
};

// It's just `sprintf` for `tr::String` lmao.
[[gnu::format(printf, 3, 4)]]
String sprintf(Ref<Arena> arena, usize maxlen, const char* fmt, ...);

}

#endif
