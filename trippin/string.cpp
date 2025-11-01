/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/string.cpp
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

#include "trippin/string.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>

#include "trippin/bits/macros.h"
#include "trippin/common.h"
#include "trippin/log.h"
#include "trippin/math.h"
#include "trippin/memory.h"
#include "trippin/thirdparty/utf8proc/utf8proc.c" // i love the preprocessor
#include "trippin/thirdparty/utf8proc/utf8proc.h"

// FIXME theres probably 2050 different violations of strict aliasing
// and 2050 different security vulnerabilities

void tr::strlib::explicit_memset(void* ptr, usize len, byte val)
{
	TR_ASSERT(ptr);
	volatile byte* p = static_cast<volatile byte*>(ptr);
	for (usize i = 0; i < len; i++) {
		p[i] = val;
	}
}

usize tr::strlib::sprintf_len(const char* fmt, va_list arg)
{
	va_list arglen;
	va_copy(arglen, arg);

#ifdef _WIN32
	int size = _vscprintf(fmt, arglen);
#else
	int size = vsnprintf(nullptr, 0, fmt, arglen);
#endif
	va_end(arglen);

	return static_cast<usize>(size);
}

tr::String::String(tr::Arena& arena, const char16* str, usize len)
{
	TR_TODO();
}

tr::String::String(tr::Arena& arena, const char32* str, usize len)
{
	TR_TODO();
}

usize tr::String::codepoint_len() const
{
	TR_TODO();
}

tr::Maybe<char32> tr::String::try_get_codepoint(usize idx) const
{
	TR_TODO();
}

char32 tr::String::get_codepoint(usize idx) const
{
	TR_TODO();
}

tr::ArrayItem<char32> tr::String::Iterator::operator*() const
{
	int32 codepoint;
	utf8proc_iterate(
		reinterpret_cast<const uint8*>(_ptr), static_cast<isize>(_idx), &codepoint
	);

	// invalid codepoints
	if (codepoint == -1) [[unlikely]] {
		codepoint = U'�'; // replacement character
	}
	TR_ASSERT(codepoint > -1);
	return {_idx, static_cast<char32>(codepoint)};
}

tr::String::Iterator& tr::String::Iterator::operator++()
{
	// utf8proc doesn't handle passing a nullptr
	[[maybe_unused]]
	int32 codepoint;
	isize read = utf8proc_iterate(
		reinterpret_cast<const uint8*>(_ptr), static_cast<isize>(_idx), &codepoint
	);

	// invalid codepoints
	if (codepoint == -1) [[unlikely]] {
		codepoint = U'�'; // replacement character
		// there must be some better way to handle this for when the codepoint is invalid
		// but i cant be bothered now
		_ptr++;
	}
	else {
		_ptr += read;
	}
	TR_ASSERT(codepoint > -1);
	return *this;
}

bool tr::String::operator==(tr::String other) const
{
	if (**this == nullptr || *other == nullptr) [[unlikely]] {
		return **this == *other;
	}
	if (len() != other.len()) {
		return false;
	}
	return memcmp(**this, *other, len()) == 0;
}

tr::Array<char32> tr::String::to_utf32(tr::Arena& arena) const
{
	TR_TODO();
}

tr::Array<char16> tr::String::to_utf16(tr::Arena& arena) const
{
	TR_TODO();
}

tr::String tr::String::substr(tr::Arena& arena, usize start, usize end) const
{
	_validate();
	if (end == 0) {
		end = len();
	}
	else {
		end = tr::clamp(end, start, len());
	}

	char* newstr = arena.alloc<char*>(end - start + 1);
	memcpy(newstr, **this + start, end);
	tr::strlib::explicit_memset(newstr + end, 1, 0);
	return {newstr, end - start};
}

tr::Array<usize> tr::String::find(tr::Arena& arena, char c, usize start, usize end) const
{
	_validate();
	if (len() == 0) {
		return {};
	}
	if (end == 0) {
		end = len();
	}
	end = tr::clamp(end, start, len()) + 1;

	Array<usize> indexes{arena};
	for (usize i = start; i < end; i++) {
		if (**this[i] == c) {
			indexes.add(i);
		}
	}
	return indexes;
}

tr::Array<usize> tr::String::find(tr::Arena& arena, tr::String str, usize start, usize end) const
{
	_validate();
	str._validate();
	if (len() == 0) {
		return {};
	}
	if (str.len() == 0) {
		return {};
	}
	if (end == 0) {
		end = len();
	}
	end = tr::clamp(end, start, len()) + 1;

	Array<usize> indexes{arena};
	for (usize i = start; i < end; i++) {
		if (memcmp(&buf()[i], *str, str.len()) == 0) {
			indexes.add(i);
		}
	}
	return indexes;
}

tr::String tr::String::concat(tr::Arena& arena, tr::String other) const
{
	_validate();
	other._validate();

	StringBuilder out{arena, len() + other.len()};
	memcpy(*out, **this, len());
	memcpy(*out + len(), *other, other.len());
	tr::strlib::explicit_memset(*out + len() + other.len() + 1, 1, 0);
	return out;
}

bool tr::String::starts_with(tr::String str) const
{
	_validate();
	str._validate();

	usize substr_len = tr::clamp(str.len(), 0u, len());
	return *this == String{*str, substr_len};
}

bool tr::String::ends_with(tr::String str) const
{
	_validate();
	str._validate();

	usize substr_len = tr::clamp(str.len(), 0u, len());
	return String{**this + len() - substr_len, substr_len} == String{*str, substr_len};
}

tr::String tr::String::file(tr::Arena& arena) const
{
	_validate();

	// this is actually safe since it'll overflow and become massive
	// TODO consider not
	for (usize i = len() - 1; i < len(); i--) {
		if (**this[i] == '/' || **this[i] == '\\') {
			return substr(arena, i + 1, len());
		}
	}

	// nothing found
	return duplicate(arena);
}

tr::String tr::String::directory(tr::Arena& arena) const
{
	_validate();

	// this is actually safe since it'll overflow and become massive
	// TODO consider not
	for (usize i = len() - 1; i < len(); i--) {
		if (**this[i] == '/' || **this[i] == '\\') {
			return substr(arena, 0, i);
		}
	}

	// nothing found
	return duplicate(arena);
}

tr::String tr::String::extension(tr::Arena& arena) const
{
	_validate();
	String file = this->file(arena);

	for (usize i = 0; i < file.len(); i++) {
		if (file[i] == '.') {
			// a . prefix is a hidden file in unix, not an extension
			if (i == 0) {
				continue;
			}

			return file.substr(arena, i, file.len());
		}
	}

	// no extension
	return {};
}

bool tr::String::is_absolute() const
{
	_validate();

	if (starts_with("/")) { // unix root
		return true;
	}
	if (starts_with("~/")) { // unix home
		return true;
	}
	if (starts_with("./")) { // relative dir
		return false;
	}
	if (starts_with("../")) { // parent dir
		return false;
	}
	// same thing but for windows
	if (starts_with(".\\")) {
		return false;
	}
	if (starts_with("..\\")) {
		return false;
	}

	// handle both windows drives and URI schemes
	// they're both some letters followed by `:/`
	for (usize i = 0; i < len(); i++) {
		char c = **this[i];
		// just ascii bcuz i doubt there's an uri scheme like lösarquívoççs://
		if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
			// pls don't undefine behavior all over the place
			if (len() > i + 2) {
				if (**this[i + 1] == ':' &&
				    (**this[i + 2] == '/' || **this[i + 2] == '\\')) {
					return true;
				}
			}
		}
	}

	return false;
}

tr::String tr::String::replace(tr::Arena& arena, char from, char to) const
{
	_validate();
	if (len() == 0) {
		return {};
	}

	StringBuilder newstr{arena, *this};
	for (usize i = 0; i < len(); i++) {
		if (newstr[i] == from) {
			newstr[i] = to;
		}
	}
	return newstr;
}

tr::Array<tr::String> tr::String::split(tr::Arena& arena, char delimiter) const
{
	// FIXME this might be ass
	_validate();
	if (len() == 0) {
		return {};
	}

	Array<String> strs{arena};
	usize last_str = 0;

	for (usize i = 0; i < len(); i++) {
		if (**this[i] == delimiter || i == len() - 1) {
			// small hack so that i don't have to change this code any more than what is
			// required to make it work
			if (i == len() - 1) {
				i++;
			}

			String sub = substr(arena, last_str, i - last_str);
			strs.add(sub);
			last_str = i + 1;

			if (i == len()) {
				return strs;
			}
		}
	}

	// if nothing was found
	if (strs.len() == 0) {
		strs.add(duplicate(arena));
	}
	return strs;
}

tr::ArrayItem<char32> tr::StringBuilder::Iterator::operator*() const
{
	int32 codepoint;
	utf8proc_iterate(
		reinterpret_cast<const uint8*>(_ptr), static_cast<isize>(_idx), &codepoint
	);

	// invalid codepoints
	if (codepoint == -1) [[unlikely]] {
		codepoint = U'�'; // replacement character
	}
	TR_ASSERT(codepoint > -1);
	return {_idx, static_cast<char32>(codepoint)};
}

tr::StringBuilder::Iterator& tr::StringBuilder::Iterator::operator++()
{
	// utf8proc doesn't handle passing a nullptr
	[[maybe_unused]]
	int32 codepoint;
	isize read = utf8proc_iterate(
		reinterpret_cast<const uint8*>(_ptr), static_cast<isize>(_idx), &codepoint
	);

	// invalid codepoints
	if (codepoint == -1) [[unlikely]] {
		codepoint = U'�'; // replacement character
		// there must be some better way to handle this for when the codepoint is invalid
		// but i cant be bothered now
		_ptr++;
	}
	else {
		_ptr += read;
	}
	TR_ASSERT(codepoint > -1);
	return *this;
}

bool tr::StringBuilder::operator==(tr::String other) const
{
	if (**this == nullptr || *other == nullptr) [[unlikely]] {
		return **this == *other;
	}
	if (len() != other.len()) {
		return false;
	}
	return memcmp(**this, *other, len()) == 0;
}

void tr::StringBuilder::append(char c)
{
	_array[len()] = c;
	_array.add('\0');
}

void tr::StringBuilder::append(tr::String s)
{
	// just in case
	if (s.len() == 0) {
		return;
	}

	_array.reserve(s.len());
	for (usize i = 0; i < s.len(); i++) {
		append((*s)[i]);
	}
}

void tr::StringBuilder::appendf(const char* fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);

	// mild evilness
	usize len = tr::strlib::sprintf_len(fmt, arg);
	char* tmpstr = tr::scratchpad().alloc<char*>(len + 1);
	std::vsnprintf(tmpstr, len + 1, fmt, arg);
	append(String{tmpstr, len});

	va_end(arg);
}

tr::String tr::fmt_args(tr::Arena& arena, const char* fmt, va_list arg)
{
	usize size = tr::strlib::sprintf_len(fmt, arg);
	// the string constructor handles the null terminator shut up
	StringBuilder str{arena, static_cast<usize>(size)};

#ifdef _WIN32
	vsnprintf_s(str.buf(), size + 1, _TRUNCATE, fmt, arg);
#else
	vsnprintf(str.buf(), static_cast<usize>(size) + 1, fmt, arg);
#endif

	// just in case
	str[static_cast<usize>(size)] = '\0';

	return str;
}

[[gnu::format(printf, 2, 3)]]
tr::String tr::fmt(tr::Arena& arena, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	String str = tr::fmt_args(arena, fmt, args);
	va_end(args);
	return str;
}
