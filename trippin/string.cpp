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

#include "trippin/common.h"
#include "trippin/log.h"
#include "trippin/math.h"
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

bool tr::strlib::strs_equal(const byte* a, usize a_len, const byte* b, usize b_len)
{
	if (a == nullptr || b == nullptr) [[unlikely]] {
		return a == b;
	}
	if (a == b) [[unlikely]] {
		return true;
	}
	if (a_len != b_len) {
		return false;
	}
	return memcmp(a, b, a_len) == 0;
}

void tr::strlib::substr(const byte* s, usize len, usize start, usize end, byte* out, usize ch_len)
{
	TR_ASSERT(s);
	TR_ASSERT(out);

	end = tr::clamp(end, start, len);
	memcpy(out, s + start, end);
	tr::strlib::explicit_memset(out + end, ch_len, 0);
}

tr::Array<usize> tr::strlib::find_char(
	tr::Arena& arena, const byte* s, usize len, usize start, usize end, const byte* ch,
	usize ch_len
)
{
	TR_ASSERT(s);
	TR_ASSERT(ch);

	if (end == 0) {
		end = len;
	}
	end = tr::clamp(end, start, len) + 1;
	// you can't find if it's not there
	if (len == 0) {
		return {};
	}

	Array<usize> indexes{arena};
	for (usize i = start; i < end; i += ch_len) {
		if (memcmp(&s[i * ch_len], ch, ch_len) == 0) {
			indexes.add(i);
		}
	}
	return indexes;
}

tr::Array<usize> tr::strlib::find_str(
	Arena& arena, const byte* s, usize len, usize start, usize end, const byte* substr,
	usize substr_len
)
{
	TR_ASSERT(s);
	TR_ASSERT(substr);

	if (end == 0) {
		end = len;
	}
	end = tr::clamp(end, start, len) + 1;
	// you can't find if it's not there
	if (len == 0) {
		return {};
	}

	Array<usize> indexes{arena};
	for (usize i = start; i < end; i++) {
		if (memcmp(&s[i], substr, substr_len) == 0) {
			indexes.add(i);
		}
	}
	return indexes;
}

void tr::strlib::concat(
	const byte* a, usize a_len, const byte* b, usize b_len, byte* out, usize ch_len
)
{
	TR_ASSERT(a);
	TR_ASSERT(b);
	TR_ASSERT(out);

	memcpy(out, a, a_len);
	memcpy(out + a_len, b, b_len);
	tr::strlib::explicit_memset(out + a_len + b_len + 1, ch_len, 0);
}

bool tr::strlib::starts_with(const byte* s, usize s_len, const byte* substr, usize substr_len)
{
	TR_ASSERT(s);
	TR_ASSERT(substr);

	substr_len = tr::clamp(substr_len, 0u, s_len);
	return tr::strlib::strs_equal(s, substr_len, substr, substr_len);
}

bool tr::strlib::ends_with(const byte* s, usize s_len, const byte* substr, usize substr_len)
{
	TR_ASSERT(s);
	TR_ASSERT(substr);

	substr_len = tr::clamp(substr_len, 0u, s_len);
	return tr::strlib::strs_equal(s + s_len - substr_len, substr_len, substr, substr_len);
}

void tr::strlib::strfile(tr::Arena& arena, const char8* s, usize len, char8** out, usize* out_len)
{
	TR_ASSERT(s);
	TR_ASSERT(out);
	TR_ASSERT(out_len);

	for (usize i = len - 1; i < len; i--) {
		if (s[i] == '/' || s[i] == '\\') {
			char8* newstr = arena.alloc<char8*>(len - i + 1);
			tr::strlib::substr(
				reinterpret_cast<const byte*>(s), len, i + 1, len,
				reinterpret_cast<byte*>(newstr), 1
			);
			*out = newstr;
			*out_len = len - i;
			return;
		}
	}

	// just duplicate the string
	char8* newstr = arena.alloc<char8*>(len + 1);
	memcpy(newstr, s, len);
	newstr[len] = '\0';
	*out = newstr;
	*out_len = len;
}

void tr::strlib::strdir(tr::Arena& arena, const char8* s, usize len, char8** out, usize* out_len)
{
	TR_ASSERT(s);
	TR_ASSERT(out);
	TR_ASSERT(out_len);

	for (usize i = len - 1; i < len; i--) {
		if (s[i] == '/' || s[i] == '\\') {
			char8* newstr = arena.alloc<char8*>(i + 1);
			tr::strlib::substr(
				reinterpret_cast<const byte*>(s), len, 0, i,
				reinterpret_cast<byte*>(newstr), sizeof(char8)
			);
			*out = newstr;
			*out_len = i;
			return;
		}
	}

	// just duplicate the string
	char8* newstr = arena.alloc<char8*>(len + 1);
	memcpy(newstr, s, len);
	newstr[len] = '\0';
	*out = newstr;
	*out_len = len;
}

void tr::strlib::strext(tr::Arena& arena, const char8* s, usize len, char8** out, usize* out_len)
{
	TR_ASSERT(s);
	TR_ASSERT(out);
	TR_ASSERT(out_len);

	char8* file;
	usize file_len;
	tr::strlib::strfile(arena, s, len, &file, &file_len);

	for (usize i = 0; i < file_len; i++) {
		if (file[i] == '.') {
			// a . prefix is a hidden file in unix, not an extension
			if (i == 0) {
				continue;
			}

			char8* newstr = arena.alloc<char8*>(file_len - i + 1);
			tr::strlib::substr(
				reinterpret_cast<const byte*>(file), file_len, i, file_len,
				reinterpret_cast<byte*>(newstr), 1
			);
			*out = newstr;
			*out_len = file_len - i;
			return;
		}
	}

	*out = nullptr;
	*out_len = 0;
}

bool tr::strlib::strabsolute(const char8* s, usize len)
{
	TR_ASSERT(s);

	// unfortunately the c-like api with c++ types makes this ugly as fuck
	const byte* sbytes = reinterpret_cast<const byte*>(s);

	const byte* unix_root = reinterpret_cast<const byte*>("/");
	if (tr::strlib::starts_with(sbytes, len, unix_root, sizeof("/") - 1)) {
		return true;
	}
	const byte* unix_home = reinterpret_cast<const byte*>("~/");
	if (tr::strlib::starts_with(sbytes, len, unix_home, sizeof("~/") - 1)) {
		return true;
	}
	const byte* unix_this_dir = reinterpret_cast<const byte*>("./");
	if (tr::strlib::starts_with(sbytes, len, unix_this_dir, sizeof("./") - 1)) {
		return false;
	}
	const byte* unix_parent_dir = reinterpret_cast<const byte*>("../");
	if (tr::strlib::starts_with(sbytes, len, unix_parent_dir, sizeof("../") - 1)) {
		return false;
	}
	const byte* win_this_dir = reinterpret_cast<const byte*>(".\\");
	if (tr::strlib::starts_with(sbytes, len, win_this_dir, sizeof(".\\") - 1)) {
		return false;
	}
	const byte* win_parent_dir = reinterpret_cast<const byte*>("..\\");
	if (tr::strlib::starts_with(sbytes, len, win_parent_dir, sizeof("..\\") - 1)) {
		return true;
	}

	// handle both windows drives and URI schemes
	// they're both some letters followed by `:/`
	for (usize i = 0; i < len; i++) {
		char8 c = s[i];
		// just ascii bcuz i doubt theres an uri scheme like lösarquívos://
		if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
			// pls don't ~~crash~~ undefine behavior all over the place
			if (len > i + 2) {
				if (s[i + 1] == ':' && (s[i + 2] == '/' || s[i + 2] == '\\')) {
					return true;
				}
			}
		}
	}

	return false;
}

void tr::strlib::replace(
	const byte* s, usize s_len, const byte* from_ch, const byte* to_ch, usize ch_len, byte* out
)
{
	TR_ASSERT(s);
	TR_ASSERT(from_ch);
	TR_ASSERT(to_ch);
	TR_ASSERT(out);

	for (usize i = 0; i < s_len; i += ch_len) {
		if (memcmp(&s[i], from_ch, ch_len) == 0) {
			memcpy(&out[i], to_ch, ch_len);
		}
		else {
			memcpy(&out[i], &s[i], ch_len);
		}
	}
}

tr::Array<byte*> tr::strlib::split_by_char(
	tr::Arena& arena, const byte* s, usize s_len, const byte* ch, usize ch_len
)
{
	Array<byte*> strs{arena};
	usize last_str = 0;

	// FIXME this WILL break

	for (usize i = 0; i < s_len; i += ch_len) {
		if (memcmp(&s[i], ch, ch_len) == 0 || i == s_len - 1) {
			// small hack so that i don't have to change this code any more than what is
			// required to make it work
			if (i == s_len - 1) {
				s_len += ch_len;
				i += ch_len;
			}

			byte* newstr = arena.alloc<byte*>(i - last_str + ch_len);
			memcpy(newstr, &s[last_str], i - last_str);
			tr::strlib::explicit_memset(newstr + i - last_str, ch_len, 0);
			strs.add(newstr);
			last_str = i + ch_len;

			if (i == s_len - 1) {
				return strs;
			}
		}
	}

	// if nothing was found
	if (strs.len() == 0) {
		byte* newstr = arena.alloc<byte*>(s_len + ch_len);
		memcpy(newstr, s, s_len);
		tr::strlib::explicit_memset(newstr + last_str, ch_len, 0);
		strs.add(newstr);
	}

	return strs;
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

tr::String16 tr::utf8_to_utf16(tr::Arena& arena, tr::String8 src)
{
	(void)arena;
	(void)src;
	TR_TODO();
}

tr::String8 tr::utf16_to_utf8(tr::Arena& arena, tr::String16 src)
{
	(void)arena;
	(void)src;
	TR_TODO();
}

tr::String32 tr::utf8_to_utf32(tr::Arena& arena, tr::String8 src)
{
	(void)arena;
	(void)src;
	TR_TODO();
}

tr::String8 tr::utf32_to_utf8(tr::Arena& arena, tr::String32 src)
{
	(void)arena;
	(void)src;
	TR_TODO();
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
