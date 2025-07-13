/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/log.cpp
 * Logging, assert, and panic
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

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <signal.h>
#ifdef _WIN32
	#include <intrin.h>
#endif

#include "common.hpp"
#include "log.hpp"
#include "memory.hpp"
#include "iofs.hpp"

namespace tr {
	extern Arena core_arena;
	extern Array<File*> logfiles;
	void __log(const char* color, const char* prefix, bool panic, const char* fmt, va_list arg);
}

void tr::use_log_file(const char* path)
{
	Result<File*, FileError> f = File::open(core_arena, path, FileMode::WRITE_TEXT);
	if (!f.is_valid()) {
		tr::warn("couldn't use log file '%s': %s", path, f.unwrap_err().message().buf());
	}
	File* file = f.unwrap();
	logfiles.add(file);

	tr::info("using log file \"%s\"", path);
}

void tr::__log(const char* color, const char* prefix, bool panic, const char* fmt, va_list arg)
{
	// you understand mechanical hands are the ruler of everything (ah)
	// TODO tr::time?? idfk
	char timestr[32];
	time_t now = time(nullptr);
	// FUCK ME
	#ifdef _WIN32
	struct tm tm_info;
	localtime_s(&tm_info, &now);
	#else
	struct tm* tm_info = localtime(&now);
	#endif
	strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", &tm_info);

	va_list argmaballs;
	va_copy(argmaballs, arg);
	String buf = tr::sprintf_args(tr::scratchpad(), fmt, argmaballs);
	va_end(argmaballs);

	for (auto [_, file] : tr::logfiles) {
		if (file->is_std) file->write_string(color);
		file->printf("[%s] %s%s", timestr, prefix, buf.buf());
		if (file->is_std) file->write_string(ConsoleColor::RESET);
		file->write_string("\n");

		file->flush();
	}

	if (panic) {
		tr::free();
		
		#ifdef _WIN32
		__debugbreak();
		#else
		__builtin_trap();
		#endif
	}
}

[[gnu::format(printf, 1, 2)]]
void tr::log(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	__log("", "", false, fmt, args);
	va_end(args);
}

[[gnu::format(printf, 1, 2)]]
void tr::info(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	__log(tr::ConsoleColor::INFO, "", false, fmt, args);
	va_end(args);
}

[[gnu::format(printf, 1, 2)]]
void tr::warn(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	__log(tr::ConsoleColor::WARN, "", false, fmt, args);
	va_end(args);
}

[[gnu::format(printf, 1, 2)]]
void tr::error(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	__log(tr::ConsoleColor::ERROR, "", false, fmt, args);
	va_end(args);
}

[[noreturn, gnu::format(printf, 1, 2)]]
void tr::panic(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	__log(tr::ConsoleColor::ERROR, "panic: ", true, fmt, args);
	// Function declared 'noreturn' should not return
	// (this will never happen because __log panics first)
	exit(1);
	va_end(args);
}

[[gnu::format(printf, 4, 5)]]
void tr::__impl_assert(const char* file, int line, bool x, const char* fmt, ...)
{
	if (!x) {
		va_list args;
		va_start(args, fmt);
		char prefix[256];
		snprintf(prefix, sizeof(prefix), "at %s:%i: ", file, line);
		__log(tr::ConsoleColor::ERROR, prefix, true, fmt, args);
		va_end(args);
	}
}
