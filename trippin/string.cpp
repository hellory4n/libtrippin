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

#include <stdarg.h>
#include <stdio.h>

#include "math.hpp"

#include "string.hpp"

// TODO maybe use less <string.h> bcuz it can get fucky

bool tr::String::operator==(const tr::String& other) const
{
	return strncmp(*this, other, tr::max(this->len(), other.len())) == 0;
}

tr::String tr::String::substr(tr::Arena& arena, usize start, usize end) const
{
	String str = String(this->buf() + start, end + 1).duplicate(arena);
	str[end] = '\0';
	return str;
}

tr::Array<usize> tr::String::find(tr::Arena& arena, tr::String str, usize start, usize end) const
{
	if (end == 0 || end > this->len()) end = this->len();

	Array<usize> indexes(tr::scratchpad(), 0);

	for (usize i = start; i < end; i++) {
		String substr = this->substr(tr::scratchpad(), i, str.len());
		if (substr == str) {
			indexes.add(i);
		}
	}

	return Array<usize>(arena, indexes.buf(), indexes.len());
}

tr::String tr::String::concat(tr::Arena& arena, tr::String other) const
{
	String new_str(arena, this->buf(), this->len() + other.len());
	// msvc is a little bitch
	#ifdef TR_ONLY_MSVC
	errno_t ohno = strncat_s(new_str.buf(), new_str.len() + 1, other.buf(), other.len());
	TR_ASSERT(ohno == 0);
	#else
	strncat(new_str.buf(), other.buf(), other.len());
	#endif
	return new_str;
}

bool tr::String::starts_with(tr::String str) const
{
	return String(this->buf(), str.len()) == str;
}

bool tr::String::ends_with(tr::String str) const
{
	return String(this->buf() + this->len() - str.len(), str.len()) == str;
}

tr::String tr::String::file(Arena& arena) const
{
	for (usize i = this->len() - 1; i < this->len(); i--) {
		if ((*this)[i] == '/' || (*this)[i] == '\\') {
			return this->substr(arena, i + 1, this->len() + 1);
		}
	}
	return this->duplicate(arena);
}

tr::String tr::String::directory(Arena& arena) const
{
	for (usize i = this->len() - 1; i < this->len(); i--) {
		if ((*this)[i] == '/' || (*this)[i] == '\\') {
			return this->substr(arena, 0, i);
		}
	}
	return this->duplicate(arena);
}

tr::String tr::String::extension(Arena& arena) const
{
	String filename = this->file(arena);
	for (usize i = 0; i < filename.len(); i++) {
		if (filename[i] == '.') {
			// a . prefix is a hidden file in unix, not an extension
			if (i == 0) return "";
			return filename.substr(arena, i, filename.len() + 1);
		}
	}
	return "";
}

bool tr::String::is_absolute() const
{
	if (this->starts_with("/"))    return true;
	if (this->starts_with("~/"))   return true;
	if (this->starts_with("./"))   return false;
	if (this->starts_with("../"))  return false;
	// i think windows supports those? lmao
	if (this->starts_with(".\\"))  return false;
	if (this->starts_with("..\\")) return false;

	// handle both windows drives and URI schemes
	// they're both some letters followed by `:/`
	for (ArrayItem<char> c : *this) {
		// just ascii bcuz i doubt theres an uri scheme like lösarquívos://
		if ((c.val >= '0' && c.val <= '9') || (c.val >= 'A' && c.val <= 'Z') || (c.val >= 'a' && c.val <= 'z')) {
			// pls don't crash
			if (this->len() > c.i + 2) {
				if ((*this)[c.i + 1] == ':' && ((*this)[c.i + 2] == '/' || (*this)[c.i + 2] == '\\')) {
					return true;
				}
			}
		}
	}

	return false;
}

[[nodiscard]]
tr::String tr::String::replace(tr::Arena& arena, char from, char to) const
{
	Array<usize> indexes = this->find(tr::scratchpad(), from);
	String str = this->duplicate(tr::scratchpad());

	for (ArrayItem<usize> c : indexes) {
		str[c.val] = to;
	}

	return str.duplicate(arena);
}

[[nodiscard]]
tr::Array<tr::String> tr::String::split(tr::Arena& arena, char delimiter) const
{
	Array<String> strings(tr::scratchpad());
	String str = this->duplicate(tr::scratchpad());
	char delim[2] = {delimiter, '\0'};

	// windows has strtok_s, posix has strtok_r
	// they're pretty much the same thing
	// interestingly strtok_s is optional (from c11) but also microsoft's strtok_s is different because FUCK ME
	// TODO does mingw gcc have microsoft's strtok_s?
	#ifdef _WIN32
	char* context = nullptr;
	char* token = strtok_s(str, delim, &context);
	while (token != nullptr) {
		String m = String(arena, token, strlen(token));
		strings.add(m);
		token = strtok_s(nullptr, delim, &context);
	}
	#else
	// TODO use strtok_r
	// i'm editing this on visual studio so it doesnt have it
	char* token = strtok(str, delim);
	while (token != nullptr) {
		String m = String(arena, token, strlen(token));
		strings.add(m);
		// WHY DENNIS RITCHIE WHY
		token = strtok(nullptr, delim);
	}
	#endif

	return Array<String>(arena, strings.buf(), strings.len());
}

[[gnu::format(printf, 3, 4), deprecated("specifying size is no longer necessary")]]
tr::String tr::sprintf(tr::Arena& arena, usize maxlen, const char* fmt, ...)
{
	String str(tr::scratchpad(), maxlen);
	va_list args;
	va_start(args, fmt);
	vsnprintf(str.buf(), maxlen, fmt, args);
	va_end(args);
	// just in case
	str[str.len() - 1] = '\0';

	// just so .len() is useful :D
	str = String(arena, str.buf(), strnlen(str.buf(), maxlen));

	return str;
}

tr::String tr::sprintf_args(tr::Arena& arena, const char* fmt, va_list arg)
{
	// TODO does this work on mingw gcc?

	va_list arglen;
	va_copy(arglen, arg);

	// get size
	// I LOVE WINDOWS!!!!!!!!!!
	#ifdef _WIN32
	int size = _vscprintf(fmt, arglen);
	#else
	int size = vsnprintf(nullptr, 0, fmt, arglen);
	#endif
	va_end(arglen);

	// the string constructor handles the null terminator shut up
	String str(arena, size);

	// do the formatting frfrfrfr
	va_list argfmt;
	va_copy(argfmt, arg);
	#ifdef _WIN32
	vsnprintf_s(str.buf(), size + 1, _TRUNCATE, fmt, argfmt);
	#else
	vsnprintf(str.buf(), size + 1, fmt, argfmt);
	#endif
	va_end(argfmt);

	// just in case
	str[size] = '\0';

	return str;
}

[[gnu::format(printf, 2, 3)]]
tr::String tr::sprintf(tr::Arena& arena, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	String str = tr::sprintf_args(arena, fmt, args);
	va_end(args);
	return str;
}
