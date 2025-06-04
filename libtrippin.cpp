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
		head = *head->prev.unwrap();
	}

	while (head != nullptr) {
		ArenaPage* next = head->next.is_valid() ? *head->next.unwrap() : nullptr;
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
		ArenaPage* prev_page = *this->page->prev.unwrap();
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
		ArenaPage* prev_page = *this->page->prev.unwrap();
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
