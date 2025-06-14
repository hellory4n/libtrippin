/*
* libtrippin v2.1.1
*
* Most biggest most massive standard library thing of all time
* https://github.com/hellory4n/libtrippin
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
#include <time.h>
#include <signal.h>
#include "libtrippin.hpp"

namespace tr {
	FILE* logfile;
	MemoryInfo memory_info;
}

void tr::init()
{
	tr::info("initialized libtrippin %s", tr::VERSION);
}

void tr::free()
{
	tr::info("deinitialized libtrippin");
	fclose(tr::logfile);
}

void tr::use_log_file(const char* path)
{
	tr::logfile = fopen(path, "w");
	tr::assert(tr::logfile != nullptr,
		"couldn't open %s, either the path is inaccessible or there's no permissions to write here", path);

	tr::info("using log file \"%s\"", path);
}

// TODO logging should use tr::String

static void __log(const char* color, const char* prefix, bool panic, const char* fmt, va_list arg)
{
	// you understand mechanical hands are the ruler of everything (ah)
	char timestr[32];
	time_t now = time(nullptr);
	struct tm* tm_info = localtime(&now);
	strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", tm_info);

	// TODO maybe increase in the future?
	char buf[256];
	vsnprintf(buf, sizeof(buf), fmt, arg);

	if (tr::logfile == nullptr) {
		printf(
			"%s [%s] no log file available. did you forget to call tr::init()?%s\n",
			color, timestr, tr::ConsoleColor::RESET
		);
	}
	else {
		fprintf(tr::logfile, "[%s] %s%s\n", timestr, prefix, buf);
		fflush(tr::logfile);
	}

	printf("%s[%s] %s%s%s\n", color, timestr, prefix, buf, tr::ConsoleColor::RESET);
	fflush(stdout);

	if (panic) {
		// windows doesn't have SIGTRAP (which sets a breakpoint) for some fucking reason
		// TODO there's probably a windows equivalent but i don't care enough to find that
		#ifndef _WIN32
		raise(SIGTRAP);
		#else
		raise(SIGABRT);
		#endif
	}
}

TR_LOG_FUNC(1, 2) void tr::log(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	__log("", "", false, fmt, args);
	va_end(args);
}

TR_LOG_FUNC(1, 2) void tr::info(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	__log(tr::ConsoleColor::INFO, "", false, fmt, args);
	va_end(args);
}

TR_LOG_FUNC(1, 2) void tr::warn(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	__log(tr::ConsoleColor::WARN, "", false, fmt, args);
	va_end(args);
}

TR_LOG_FUNC(1, 2) void tr::error(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	__log(tr::ConsoleColor::ERROR, "", false, fmt, args);
	va_end(args);
}

TR_LOG_FUNC(1, 2) void tr::panic(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	__log(tr::ConsoleColor::ERROR, "panic: ", true, fmt, args);
	// Function declared 'noreturn' should not return
	// (this will never happen because __log panics first)
	exit(1);
	va_end(args);
}

TR_LOG_FUNC(2, 3) void tr::assert(bool x, const char* fmt, ...)
{
	if (!x) {
		va_list args;
		va_start(args, fmt);
		__log(tr::ConsoleColor::ERROR, "failed assert: ", true, fmt, args);
		va_end(args);
	}
}

tr::Vec4<float32>& tr::Matrix4x4::operator[](usize idx)
{
	return this->values[idx];
}

tr::Matrix4x4::Matrix4x4()
{
	for (usize i = 0; i < 4; i++) {
		for (usize j = 0; j < 4; j++) {
			(*this)[i][j] = 0;
		}
	}
}

tr::Matrix4x4 tr::Matrix4x4::identity()
{
	Matrix4x4 m;
	for (usize i = 0; i < 4; i++) {
		for (usize j = 0; j < 4; j++) {
			m[i][j] = i == j ? 1 : 0;
		}
	}
	return m;
}

tr::Vec4<float32> tr::Matrix4x4::row(usize idx)
{
	Vec4<float32> r;
	for (usize k = 0; k < 4; k++) {
		r[k] = (*this)[k][idx];
	}
	return r;
}

tr::Vec4<float32> tr::Matrix4x4::column(usize idx)
{
	Vec4<float32> r;
	for (usize k = 0; k < 4; k++) {
		r[k] = (*this)[idx][k];
	}
	return r;
}

tr::Matrix4x4 tr::Matrix4x4::transpose()
{
	Matrix4x4 m;
	for (usize j = 0; j < 4; j++) {
		for (usize i = 0; i < 4; i++) {
			m[i][j] = (*this)[j][i];
		}
	}
	return m;
}

tr::Matrix4x4 tr::Matrix4x4::operator+(tr::Matrix4x4 b)
{
	Matrix4x4 m;
	for (usize i = 0; i < 4; i++) {
		m[i] = (*this)[i] + b[i];
	}
	return m;
}

tr::Matrix4x4 tr::Matrix4x4::operator-(tr::Matrix4x4 b)
{
	Matrix4x4 m;
	for (usize i = 0; i < 4; i++) {
		m[i] = (*this)[i] - b[i];
	}
	return m;
}

tr::Matrix4x4 tr::Matrix4x4::operator*(float32 b)
{
	Matrix4x4 m;
	for (usize i = 0; i < 4; i++) {
		m[i] = (*this)[i] * b;
	}
	return m;
}

tr::Matrix4x4 tr::Matrix4x4::operator*(tr::Vec3<float32> b)
{
	Matrix4x4 m;
	m[0] = (*this)[0] * b.x;
	m[1] = (*this)[1] * b.y;
	m[2] = (*this)[2] * b.z;
	m[3] = (*this)[3];
	return m;
}

tr::Matrix4x4 tr::Matrix4x4::operator*(tr::Matrix4x4 b)
{
	tr::Matrix4x4 m, temp;
	for (usize c = 0; c < 4; c++) {
		for (usize r = 0; r < 4; r++) {
			temp[c][r] = 0;
			for (usize k = 0; k < 4; k++) {
				temp[c][r] += (*this)[k][r] * b[c][k];
			}
		}
	}
	return m;
}

tr::Vec4<float32> tr::Matrix4x4::operator*(tr::Vec4<float32> b)
{
	Vec4<float32> r;
	for (usize j = 0; j < 4; j++) {
		r[j] = 0;
		for (usize i = 0; i < 4; i++) {
			r[j] += (*this)[i][j] * b[i];
		}
	}
	return r;
}

tr::Matrix4x4 tr::Matrix4x4::translate(float32 x, float32 y, float32 z)
{
	Matrix4x4 m = Matrix4x4::identity();
	m[3][0] = x;
	m[3][1] = y;
	m[3][2] = z;
	return m;
}

tr::Matrix4x4 tr::Matrix4x4::translate_in_place(float32 x, float32 y, float32 z)
{
	Matrix4x4 m = Matrix4x4::identity();
	Vec4<float32> t = Vec4<float32>(x, y, z, 0);
	for (usize i = 0; i < 4; i++) {
		Vec4<float32> r = m.row(i);
		m[3][i] += r.mul_inner(t);
	}
	return m;
}

tr::Matrix4x4 tr::Matrix4x4::from_vec3_mul_outer(tr::Vec3<float32> a, tr::Vec3<float32> b)
{
	Matrix4x4 m;
	for (usize i = 0; i < 4; i++) {
		for (usize j = 0; j < 4; j++) {
			m[i][j] = i < 3 && j < 3 ? a[i] * b[j] : 0;
		}
	}
	return m;
}

tr::Matrix4x4 tr::Matrix4x4::rotate(float32 x, float32 y, float32 z, float32 radians)
{
	float32 s = sinf(radians);
	float32 c = cosf(radians);
	Vec3<float32> u = Vec3<float32>(x, y, z);
	Matrix4x4 R;

	if (u.length() > 1e-4) {
		u = u.normalize();
		Matrix4x4 T = Matrix4x4::from_vec3_mul_outer(u, u);

		Matrix4x4 S;
		S.values[0] = {0, u[2], -u[1], 0};
		S.values[1] = {-u[2], 0, u[0], 0};
		S.values[2] = {u[1], -u[0], 0, 0},
		S.values[3] = {0, 0, 0, 0};
		S = S * s;

		Matrix4x4 C = Matrix4x4::identity();
		C = C - T;
		C = C * c;

		T = T + C;
		T = T + S;

		T[3][3] = 1;
		R = *this * T;
	} else {
		R = *this;
	}
	return R;
}

tr::Matrix4x4 tr::Matrix4x4::rotate_x(float32 radians)
{
	float32 s = sinf(radians);
	float32 c = cosf(radians);
	Matrix4x4 R;
	R[0] = {1, 0, 0, 0};
	R[1] = {0, c, s, 0};
	R[2] = {0, -s, c, 0};
	R[3] = {0, 0, 0, 1};
	return *this * R;
}

tr::Matrix4x4 tr::Matrix4x4::rotate_y(float32 radians)
{
	float32 s = sinf(radians);
	float32 c = cosf(radians);
	Matrix4x4 R;
	R[0] = {c, 0, -s, 0};
	R[1] = {0, 1, 0, 0};
	R[2] = {s, 0, c, 0};
	R[3] = {0, 0, 0, 1};
	return *this * R;
}

tr::Matrix4x4 tr::Matrix4x4::rotate_z(float32 radians)
{
	float32 s = sinf(radians);
	float32 c = cosf(radians);
	Matrix4x4 R;
	R[0] = {c, s, 0, 0};
	R[1] = {-s, c, 0, 0};
	R[2] = {0, 0, 1, 0};
	R[3] = {0, 0, 0, 1};
	return *this * R;
}

tr::Matrix4x4 tr::Matrix4x4::invert()
{
	Matrix4x4 m;
	float32 s[6];
	float32 c[6];
	s[0] = (*this)[0][0]*(*this)[1][1] - (*this)[1][0]*(*this)[0][1];
	s[1] = (*this)[0][0]*(*this)[1][2] - (*this)[1][0]*(*this)[0][2];
	s[2] = (*this)[0][0]*(*this)[1][3] - (*this)[1][0]*(*this)[0][3];
	s[3] = (*this)[0][1]*(*this)[1][2] - (*this)[1][1]*(*this)[0][2];
	s[4] = (*this)[0][1]*(*this)[1][3] - (*this)[1][1]*(*this)[0][3];
	s[5] = (*this)[0][2]*(*this)[1][3] - (*this)[1][2]*(*this)[0][3];

	c[0] = (*this)[2][0]*(*this)[3][1] - (*this)[3][0]*(*this)[2][1];
	c[1] = (*this)[2][0]*(*this)[3][2] - (*this)[3][0]*(*this)[2][2];
	c[2] = (*this)[2][0]*(*this)[3][3] - (*this)[3][0]*(*this)[2][3];
	c[3] = (*this)[2][1]*(*this)[3][2] - (*this)[3][1]*(*this)[2][2];
	c[4] = (*this)[2][1]*(*this)[3][3] - (*this)[3][1]*(*this)[2][3];
	c[5] = (*this)[2][2]*(*this)[3][3] - (*this)[3][2]*(*this)[2][3];

	float idet = 1.0f/( s[0]*c[5]-s[1]*c[4]+s[2]*c[3]+s[3]*c[2]-s[4]*c[1]+s[5]*c[0] );

	m[0][0] = ( (*this)[1][1] * c[5] - (*this)[1][2] * c[4] + (*this)[1][3] * c[3]) * idet;
	m[0][1] = (-(*this)[0][1] * c[5] + (*this)[0][2] * c[4] - (*this)[0][3] * c[3]) * idet;
	m[0][2] = ( (*this)[3][1] * s[5] - (*this)[3][2] * s[4] + (*this)[3][3] * s[3]) * idet;
	m[0][3] = (-(*this)[2][1] * s[5] + (*this)[2][2] * s[4] - (*this)[2][3] * s[3]) * idet;

	m[1][0] = (-(*this)[1][0] * c[5] + (*this)[1][2] * c[2] - (*this)[1][3] * c[1]) * idet;
	m[1][1] = ( (*this)[0][0] * c[5] - (*this)[0][2] * c[2] + (*this)[0][3] * c[1]) * idet;
	m[1][2] = (-(*this)[3][0] * s[5] + (*this)[3][2] * s[2] - (*this)[3][3] * s[1]) * idet;
	m[1][3] = ( (*this)[2][0] * s[5] - (*this)[2][2] * s[2] + (*this)[2][3] * s[1]) * idet;

	m[2][0] = ( (*this)[1][0] * c[4] - (*this)[1][1] * c[2] + (*this)[1][3] * c[0]) * idet;
	m[2][1] = (-(*this)[0][0] * c[4] + (*this)[0][1] * c[2] - (*this)[0][3] * c[0]) * idet;
	m[2][2] = ( (*this)[3][0] * s[4] - (*this)[3][1] * s[2] + (*this)[3][3] * s[0]) * idet;
	m[2][3] = (-(*this)[2][0] * s[4] + (*this)[2][1] * s[2] - (*this)[2][3] * s[0]) * idet;

	m[3][0] = (-(*this)[1][0] * c[3] + (*this)[1][1] * c[1] - (*this)[1][2] * c[0]) * idet;
	m[3][1] = ( (*this)[0][0] * c[3] - (*this)[0][1] * c[1] + (*this)[0][2] * c[0]) * idet;
	m[3][2] = (-(*this)[3][0] * s[3] + (*this)[3][1] * s[1] - (*this)[3][2] * s[0]) * idet;
	m[3][3] = ( (*this)[2][0] * s[3] - (*this)[2][1] * s[1] + (*this)[2][2] * s[0]) * idet;

	return m;
}

tr::Matrix4x4 tr::Matrix4x4::frustum(float32 left, float32 right, float32 bottom, float32 top, float32 near,
	float32 far)
{
	Matrix4x4 m;
	m[0][0] = 2.f*near/(right-left);
	m[0][1] = m[0][2] = m[0][3] = 0.f;

	m[1][1] = 2.f*near/(top-bottom);
	m[1][0] = m[1][2] = m[1][3] = 0.f;

	m[2][0] = (right+left)/(right-left);
	m[2][1] = (top+bottom)/(top-bottom);
	m[2][2] = -(far+near)/(far-near);
	m[2][3] = -1.f;

	m[3][2] = -2.f*(far*near)/(far-near);
	m[3][0] = m[3][1] = m[3][3] = 0.f;
	return m;
}

tr::Matrix4x4 tr::Matrix4x4::orthographic(float32 left, float32 right, float32 bottom, float32 top, float32 near,
	float32 far)
{
	Matrix4x4 m;
	m[0][0] = 2.f/(right-left);
	m[0][1] = m[0][2] = m[0][3] = 0.f;

	m[1][1] = 2.f/(top-bottom);
	m[1][0] = m[1][2] = m[1][3] = 0.f;

	m[2][2] = -2.f/(far-near);
	m[2][0] = m[2][1] = m[2][3] = 0.f;

	m[3][0] = -(right+left)/(right-left);
	m[3][1] = -(top+bottom)/(top-bottom);
	m[3][2] = -(far+near)/(far-near);
	m[3][3] = 1.f;
	return m;
}

tr::Matrix4x4 tr::Matrix4x4::perspective(float32 fovy, float32 aspect, float32 near, float32 far)
{
	Matrix4x4 m;
	const float32 a = 1.f / tanf(fovy / 2.f);

	m[0][0] = a / aspect;
	m[0][1] = 0.f;
	m[0][2] = 0.f;
	m[0][3] = 0.f;

	m[1][0] = 0.f;
	m[1][1] = a;
	m[1][2] = 0.f;
	m[1][3] = 0.f;

	m[2][0] = 0.f;
	m[2][1] = 0.f;
	m[2][2] = -((far + near) / (far - near));
	m[2][3] = -1.f;

	m[3][0] = 0.f;
	m[3][1] = 0.f;
	m[3][2] = -((2.f * far * near) / (far - near));
	m[3][3] = 0.f;
	return m;
}

tr::Random::Random(int64 seed)
{
	// i think this is how you implement splitmix64?
	this->state[0] = seed;
	for (size_t i = 1; i < 4; i++) {
		this->state[i] = (this->state[i - 1] += UINT64_C(0x9E3779B97F4A7C15));
		this->state[i] = (this->state[i] ^ (this->state[i] >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
		this->state[i] = (this->state[i] ^ (this->state[i] >> 27)) * UINT64_C(0x94D049BB133111EB);
		this->state[i] = this->state[i] ^ (this->state[i] >> 31);
	}
}

static inline uint64 rotl(const uint64 x, int k) {
	return (x << k) | (x >> (64 - k));
}

float64 tr::Random::next()
{
	// theft
	const uint64 result = this->state[0] + this->state[3];

	const uint64 t = this->state[1] << 17;

	this->state[2] ^= this->state[0];
	this->state[3] ^= this->state[1];
	this->state[1] ^= this->state[2];
	this->state[0] ^= this->state[3];

	this->state[2] ^= t;

	this->state[3] = rotl(this->state[3], 45);

	// not theft
	// man is 0 to 1
	// 18446744073709551616.0 is UINT64_MAX but with the last digit changed because
	// clang was complaining
	return static_cast<float64>(result) / 18446744073709551616.0;
}

void tr::RefCounted::retain() const
{
	// shut up it works
	TR_GCC_IGNORE_WARNING(-Wtautological-undefined-compare)
	// man
	if (this != nullptr) {
		// probably was just birthed into this world
		if (this->count == 0) {
			tr::memory_info.cumulative_ref_counted_objs++;
			tr::memory_info.ref_counted_objs++;
		}
		this->count++;
	}
	TR_GCC_RESTORE()
}

void tr::RefCounted::release() const
{
	if (--this->count == 0) {
		delete this;
		tr::memory_info.freed_ref_counted_objs++;
		tr::memory_info.ref_counted_objs--;
	}
}

tr::ArenaPage::ArenaPage(usize size)
{
	tr::assert(size != 0, "page size can't be 0");

	this->buffer = calloc(1, size);
	this->size = size;
	this->alloc_pos = 0;
	this->next = nullptr;
	this->prev = nullptr;

	// i don't think you can recover from that
	// so just die
	tr::assert(this->buffer != nullptr, "couldn't allocate arena page");

	// man
	tr::memory_info.alive_pages++;
	tr::memory_info.cumulative_pages++;
	tr::memory_info.cumulative_allocated += size;
	tr::memory_info.allocated += size;
}

tr::ArenaPage::~ArenaPage()
{
	if (this->buffer != nullptr) {
		// tr:: also has a function called free
		::free(this->buffer);
		this->buffer = nullptr;

		// man
		tr::memory_info.alive_pages--;
		tr::memory_info.freed_pages++;
		tr::memory_info.allocated -= this->size;
		tr::memory_info.freed_by_arenas += this->size;
	}
}

usize tr::ArenaPage::available_space()
{
	return this->size - this->alloc_pos;
}

tr::Arena::Arena(usize page_size)
{
	this->page_size = page_size;
	this->page = new ArenaPage(page_size);
}

tr::Arena::~Arena()
{
	// likely uninitialized, likely intentional
	// if it's not intentional then it'd already crashed with the other functions
	if (this->page_size == 0) return;

	ArenaPage* head = this->page;
	while (head->prev != nullptr) {
		head = head->prev;
	}

	while (head != nullptr) {
		ArenaPage* next = head->next;
		delete head;
		head = next;
	}
}

void* tr::Arena::alloc(usize size)
{
	tr::assert(this->page_size != 0, "you doofus this arena is very likely uninitialized");

	// does it fit in the current page?
	if (this->page->available_space() >= size) {
		void* val = (uint8*)this->page->buffer + this->page->alloc_pos;
		this->page->alloc_pos += size;
		return val;
	}

	// does it fit in the previous page?
	if (this->page->prev != nullptr) {
		ArenaPage* prev_page = this->page->prev;
		if (prev_page->available_space() >= size) {
			void* val = (uint8*)prev_page->buffer + prev_page->alloc_pos;
			prev_page->alloc_pos += size;
			return val;
		}
	}

	// does it fit in a regularly sized page?
	if (this->page_size >= size) {
		ArenaPage* new_page = new ArenaPage(this->page_size);
		new_page->prev = this->page;
		this->page->next = new_page;
		this->page = new_page;

		void* val = (uint8*)new_page->buffer + new_page->alloc_pos;
		new_page->alloc_pos += size;
		return val;
	}

	// last resort is making a new page with that size
	ArenaPage* new_page = new ArenaPage(size);
	new_page->prev = this->page;
	this->page->next = new_page;
	this->page = new_page;

	void* val = (uint8*)new_page->buffer + new_page->alloc_pos;
	new_page->alloc_pos += size;
	return val;
}

void tr::Arena::prealloc(usize size)
{
	tr::assert(this->page_size != 0, "you doofus this arena is very likely uninitialized");

	// does it already fit?
	if (this->page->available_space() >= size) {
		return;
	}

	if (this->page->prev != nullptr) {
		ArenaPage* prev_page = this->page->prev;
		if (prev_page->available_space() >= size) {
			return;
		}
	}

	// make a new page without increasing alloc_pos
	// i know using fmax here is questionable
	ArenaPage* new_page = new ArenaPage((usize)fmax(size, this->page_size));
	new_page->prev = this->page;
	this->page->next = new_page;
	this->page = new_page;
}

tr::MemoryInfo tr::get_memory_info()
{
	return tr::memory_info;
}

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

bool tr::String::operator==(const tr::String& other)
{
	return strncmp(*this, other, tr::max(this->length(), other.length())) == 0;
}

tr::String tr::String::substr(tr::Ref<tr::Arena> arena, usize start, usize end)
{
	String str = String(this->buffer() + start, end + 1).duplicate(arena);
	str[end] = '\0';
	return str;
}

tr::Array<usize> tr::String::find(tr::Ref<tr::Arena> arena, tr::String str, usize start, usize end)
{
	if (end == 0) end = this->length();

	Ref<Arena> tmp = new Arena(tr::kb_to_bytes(64));
	Array<usize> indexes(tmp, 0);

	for (usize i = start; i < end; i++) {
		String substr = this->substr(tmp, i, str.length());
		if (substr == str) {
			indexes.add(i);
		}
	}

	return indexes.duplicate(arena);
}

tr::String tr::String::concat(tr::Ref<tr::Arena> arena, tr::String other)
{
	String new_str(arena, this->buffer(), this->length() + other.length());
	strncat(new_str.buffer(), other.buffer(), other.length());
	return new_str;
}

bool tr::String::starts_with(tr::String str)
{
	return String(this->buffer(), str.length()) == str;
}

bool tr::String::ends_with(tr::String str)
{
	return String(this->buffer() + this->length() - str.length(), str.length()) == str;
}

tr::String tr::String::file(Ref<Arena> arena)
{
	for (usize i = this->length() - 1; i >= 0; i--) {
		if ((*this)[i] == '/' || (*this)[i] == '\\') {
			return this->substr(arena, i + 1, this->length() + 1);
		}
	}
	return this->duplicate(arena);
}

tr::String tr::String::directory(Ref<Arena> arena)
{
	for (usize i = this->length() - 1; i >= 0; i--) {
		if ((*this)[i] == '/' || (*this)[i] == '\\') {
			return this->substr(arena, 0, i);
		}
	}
	return this->duplicate(arena);
}

tr::String tr::String::extension(Ref<Arena> arena)
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

bool tr::String::is_absolute()
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
