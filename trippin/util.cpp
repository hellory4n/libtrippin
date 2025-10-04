/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/util.cpp
 * Utilities and stuff
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

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define NOSERVICE
	#define NOMCX
	#define NOIME
	// mingw gcc already defines that by default??
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
	#include <windows.h>

	// conflicts :D
	#undef ERROR
	#undef far
	#undef near
#else // posix
	#include <ctime>
#endif

#include "trippin/common.h"
#include "trippin/util.h"

constexpr uint64 FNV_OFFSET_BASIS = 0xcbf29ce484222325;
// IM IN MY PRIME™ AND THIS AINT EVEN FINAL FORM
constexpr uint64 FNV_PRIME = 0x100000001b3;

uint64 tr::hash(const uint8* bytes, usize len)
{
	uint64 hash = FNV_OFFSET_BASIS;

	for (usize i = 0; i < len; i++) {
		hash ^= bytes[i];
		hash *= FNV_PRIME;
	}

	return hash;
}

int64 tr::Stopwatch::_time_now_us()
{
#ifdef _WIN32
	LARGE_INTEGER freq, counter;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&counter);
	return (counter.QuadPart * 1000000LL) / freq.QuadPart;
#else
	struct timespec ts{};
	clock_gettime(CLOCK_REALTIME, &ts);
	return int64(ts.tv_sec) * 1000000 + ts.tv_nsec / 1000;
#endif
}

void tr::Stopwatch::start()
{
	_start = Stopwatch::_time_now_us();
}

void tr::Stopwatch::stop()
{
	_end = Stopwatch::_time_now_us();
}

int64 tr::Stopwatch::elapsed_us() const
{
	return _end - _start;
}

int64 tr::Stopwatch::elapsed_ms() const
{
	return elapsed_us() / 1000;
}

float64 tr::Stopwatch::elapsed_sec() const
{
	// TODO probably losing precision here
	return float64(elapsed_us()) / 1000.0 / 1000.0;
}

void tr::Stopwatch::print_time_sec(String label) const
{
	tr::log("%s took %fs", *label, elapsed_sec());
}

void tr::Stopwatch::print_time_ms(String label) const
{
	tr::log("%s took %li ms", *label, elapsed_ms());
}

void tr::Stopwatch::print_time_us(String label) const
{
	// windows pls dont shit yourself with a mere µ
	tr::log("%s took %li µs", *label, elapsed_us());
}
