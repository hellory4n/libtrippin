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
#include "collection.hpp"

#include "string.hpp"

TR_LOG_FUNC(3, 4) tr::String tr::sprintf(Ref<Arena> arena, usize maxlen, const char* fmt, ...)
{
	String str(arena, maxlen);
	va_list args;
	va_start(args, fmt);
	vsnprintf(str.buffer(), maxlen, fmt, args);
	va_end(args);
	// just in case
	str[str.length() - 1] = '\0';
	return str;
}

bool tr::String::operator==(const tr::String& other) const
{
	return strncmp(*this, other, tr::max(this->length(), other.length())) == 0;
}

tr::String tr::String::substr(tr::Ref<tr::Arena> arena, usize start, usize end) const
{
	// shut up asan
	String str = String(this->buffer() + start, end + 1)
		.duplicate(arena);
	str[end] = '\0';
	return str;
}

tr::Array<usize> tr::String::find(tr::Ref<tr::Arena> arena, tr::String str, usize start, usize end) const
{
	if (end == 0 || end > this->length()) end = this->length();

	Ref<Arena> tmp = new Arena(tr::kb_to_bytes(64));
	List<usize> indexes;

	for (usize i = start; i < end; i++) {
		String substr = this->substr(tmp, i, str.length());
		if (substr == str) {
			indexes.add(i);
		}
	}

	return Array<usize>(arena, indexes.buffer(), indexes.length());
}

tr::String tr::String::concat(tr::Ref<tr::Arena> arena, tr::String other) const
{
	String new_str(arena, this->buffer(), this->length() + other.length());
	strncat(new_str.buffer(), other.buffer(), other.length());
	return new_str;
}

bool tr::String::starts_with(tr::String str) const
{
	return String(this->buffer(), str.length()) == str;
}

bool tr::String::ends_with(tr::String str) const
{
	return String(this->buffer() + this->length() - str.length(), str.length()) == str;
}

tr::String tr::String::file(Ref<Arena> arena) const
{
	for (usize i = this->length() - 1; i < this->length(); i--) {
		if ((*this)[i] == '/' || (*this)[i] == '\\') {
			return this->substr(arena, i + 1, this->length() + 1);
		}
	}
	return this->duplicate(arena);
}

tr::String tr::String::directory(Ref<Arena> arena) const
{
	for (usize i = this->length() - 1; i < this->length(); i--) {
		if ((*this)[i] == '/' || (*this)[i] == '\\') {
			return this->substr(arena, 0, i);
		}
	}
	return this->duplicate(arena);
}

tr::String tr::String::extension(Ref<Arena> arena) const
{
	String filename = this->file(arena);
	for (usize i = 0; i < filename.length(); i++) {
		if (filename[i] == '.') {
			// a . prefix is a hidden file in unix, not an extension
			if (i == 0) return "";
			return filename.substr(arena, i, filename.length() + 1);
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
			if (this->length() > c.i + 2) {
				if ((*this)[c.i + 1] == ':' && ((*this)[c.i + 2] == '/' || (*this)[c.i + 2] == '\\')) {
					return true;
				}
			}
		}
	}

	return false;
}

[[nodiscard]] tr::String tr::String::replace(tr::Ref<tr::Arena> arena, char from, char to) const
{
	Ref<Arena> tmp = new Arena(this->length() + 1);
	Array<usize> indexes = this->find(tmp, from);
	String str = this->duplicate(tmp);

	for (ArrayItem<usize> c : indexes) {
		str[c.val] = to;
	}

	return str.duplicate(arena);
}

[[nodiscard]] tr::Array<tr::String> tr::String::split(tr::Ref<tr::Arena> arena, char delimiter) const
{
	Ref<List<String>> strings = new List<String>();
	String str = this->duplicate(arena);
	char delim[2] = {delimiter, '\0'};

	char* token = strtok(str, delim);
	while (token != nullptr) {
		String m = String(arena, token, strlen(token));
		strings->add(m);
		tr::log("%s", token);
		// WHY DENNIS RITCHIE WHY
		token = strtok(nullptr, delim);
	}

	return Array<String>(arena, strings->buffer(), strings->length());
}
