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
#include "trippin/memory.h"
#include "trippin/thirdparty/utf8proc/utf8proc.c" // i love the preprocessor

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

tr::String::Iterator::Iterator(const char* pointer, usize index)
{
	TR_TODO();
}

tr::ArrayItem<char32> tr::String::Iterator::operator*() const
{
	TR_TODO();
}

tr::String::Iterator& tr::String::Iterator::operator++()
{
	TR_TODO();
}

bool tr::String::operator==(tr::String other) const
{
	TR_TODO();
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
	TR_TODO();
}

tr::Array<usize> tr::String::find(tr::Arena& arena, char c, usize start, usize end) const
{
	TR_TODO();
}

tr::Array<usize> tr::String::find(tr::Arena& arena, tr::String str, usize start, usize end) const
{
	TR_TODO();
}

tr::String tr::String::concat(tr::Arena& arena, tr::String other) const
{
	TR_TODO();
}

bool tr::String::starts_with(tr::String str) const
{
	TR_TODO();
}

bool tr::String::ends_with(tr::String str) const
{
	TR_TODO();
}

tr::String tr::String::file(tr::Arena& arena) const
{
	TR_TODO();
}

tr::String tr::String::directory(tr::Arena& arena) const
{
	TR_TODO();
}

tr::String tr::String::extension(tr::Arena& arena) const
{
	TR_TODO();
}

bool tr::String::is_absolute() const
{
	TR_TODO();
}

tr::String tr::String::replace(tr::Arena& arena, char from, char to) const
{
	TR_TODO();
}

tr::Array<tr::String> tr::String::split(tr::Arena& arena, char delimiter) const
{
	TR_TODO();
}

tr::StringBuilder::Iterator::Iterator(const char* pointer, usize index)
{
	TR_TODO();
}

tr::ArrayItem<char&> tr::StringBuilder::Iterator::operator*() const
{
	TR_TODO();
}

tr::StringBuilder::Iterator& tr::StringBuilder::Iterator::operator++()
{
	TR_TODO();
}

bool tr::StringBuilder::operator==(tr::String other) const
{
	TR_TODO();
}

void tr::StringBuilder::append(char c)
{
	TR_TODO();
}

void tr::StringBuilder::append(tr::String s)
{
	TR_TODO();
}

void tr::StringBuilder::appendf(const char* fmt, ...)
{
	TR_TODO();
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
