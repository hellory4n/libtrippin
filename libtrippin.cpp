/*
* libtrippin v2.0.0
*
* Most biggest most massive standard library thing for C of all time
* More information at https://github.com/hellory4n/libtrippin
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

#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <csignal>
#include "libtrippin.hpp"

namespace tr {
	FILE* logfile;
}

void tr::init()
{
	tr::liblog("initialized libtrippin %s", tr::VERSION);
}

void tr::free()
{
	tr::liblog("deinitialized libtrippin");
	fclose(tr::logfile);
}

void tr::use_log_file(const char* path)
{
	tr::logfile = fopen(path, "w");
	tr::assert(tr::logfile != nullptr,
		"couldn't open %s, either the path is inaccessible or there's no permissions to write here", path);

	tr::liblog("using log file \"%s\"", path);
}

static void __log(const char* color, const char* prefix, bool panic, const char* fmt, va_list arg)
{
	// you understand mechanical hands are the ruler of everything (ah)
	char timestr[32];
	time_t now = time(nullptr);
	tm* tm_info = localtime(&now);
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

TR_LOG_FUNC(1, 2) void tr::liblog(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	__log(tr::ConsoleColor::LIB_INFO, "", false, fmt, args);
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
	#ifdef DEBUG
	if (!x) {
		va_list args;
		va_start(args, fmt);
		__log(tr::ConsoleColor::ERROR, "failed assert: ", true, fmt, args);
		va_end(args);
	}
	#else
	// the compiler is complaining about unused arguments
	(void)x;
	(void)fmt;
	#endif
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

	if (u.len() > 1e-4) {
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

tr::Matrix4x4 tr::Matrix4x4::ortho(float32 left, float32 right, float32 bottom, float32 top, float32 near,
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

tr::ArenaPage::ArenaPage(usize size)
{
	this->buffer = malloc(size);
	this->size = size;
	this->alloc_pos = 0;
	this->next = nullptr;
	this->prev = nullptr;

	// i don't think you can recover from that
	// so just die
	tr::assert(this->buffer != nullptr, "couldn't allocate arena page");
}

tr::ArenaPage::~ArenaPage()
{
	if (this->buffer != nullptr) {
		// full legal name because tr:: also has a function called free
		std::free(this->buffer);
		this->buffer = nullptr;
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
	ArenaPage* head = this->page;
	while (head->prev.is_valid()) {
		head = head->prev.unwrap();
	}

	while (head != nullptr) {
		ArenaPage* next = head->next.is_valid() ? head->next.unwrap() : nullptr;
		delete head;
		head = next;
	}
}

void* tr::Arena::alloc(usize size)
{
	// does it fit in the current page?
	if (this->page->available_space() >= size) {
		void* val = (char*)this->page->buffer + this->page->alloc_pos;
		this->page->alloc_pos += size;
		return val;
	}

	// does it fit in the previous page?
	if (this->page->prev.is_valid()) {
		ArenaPage* prev_page = this->page->prev.unwrap();
		if (prev_page->available_space() >= size) {
			void* val = (char*)prev_page->buffer + prev_page->alloc_pos;
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

		void* val = (char*)new_page->buffer + new_page->alloc_pos;
		new_page->alloc_pos += size;
		return val;
	}

	// last resort is making a new page with that size
	ArenaPage* new_page = new ArenaPage(size);
	new_page->prev = this->page;
	this->page->next = new_page;
	this->page = new_page;

	void* val = (char*)new_page->buffer + new_page->alloc_pos;
	new_page->alloc_pos += size;
	return val;
}

void tr::Arena::prealloc(usize size)
{
	// does it already fit?
	if (this->page->available_space() >= size) {
		return;
	}

	if (this->page->prev.is_valid()) {
		ArenaPage* prev_page = this->page->prev.unwrap();
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
	return (float64)result / 18446744073709551616.0;
}
