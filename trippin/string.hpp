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
class String {
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

	// man
	char& operator[](usize idx) const { return this->array[idx]; }
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
	bool operator==(const String& other);
	bool operator!=(const String& other) { return !(*this == other); }
	bool operator==(const char* other)   { return *this == String(other); }
	bool operator!=(const char* other)   { return *this != String(other); }

	// Gets a substring. The returned string doesn't include the end character.
	String substr(Ref<Arena> arena, usize start, usize end);

	// Returns an array with all of the indexes
	Array<usize> find(Ref<Arena> arena, String str, usize start = 0, usize end = 0);

	// It concatenates 2 strings lmao.
	String concat(Ref<Arena> arena, String other);

	// If true, the string starts with that other crap.
	bool starts_with(String str);

	// If true, the string ends with that other crap.
	bool ends_with(String str);

	// Gets the filename in a path, e.g. returns `file.txt` for `/path/to/file.txt`
	String file(Ref<Arena> arena);

	// Gets the directory in a path e.g. returns `/path/to` for `/path/to/file.txt`
	String directory(Ref<Arena> arena);

	// Returns the extension in a path, e.g. returns `.txt` for `/path/to/file.txt`, `.blend.1` for
	// `teapot.blend.1`, and an empty string for `.gitignore`
	String extension(Ref<Arena> arena);

	// If true, the path is absolute. Else, it's relative.
	bool is_absolute();
};

// It's just `sprintf` for `tr::String` lmao.
TR_LOG_FUNC(3, 4) String sprintf(Ref<Arena> arena, usize maxlen, const char* fmt, ...);

}

#endif
