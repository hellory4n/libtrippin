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

#include "common.hpp"
#include "log.hpp"

// TODO logging should use tr::String and tr::File

namespace tr {
	extern FILE* logfile;
}

void tr::use_log_file(const char* path)
{
	tr::logfile = fopen(path, "w");
	TR_ASSERT_MSG(tr::logfile != nullptr,
		"couldn't open %s, either the path is inaccessible or there's no permissions to write here", path);

	tr::info("using log file \"%s\"", path);
}

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
		tr::free();
		// TODO signal for when this happens
		// so that everything (starry3d) can safely close :)

		// windows doesn't have SIGTRAP (which sets a breakpoint) for some fucking reason
		// TODO there's probably a windows equivalent but i don't care enough to find that
		#ifndef _WIN32
		raise(SIGTRAP);
		#else
		raise(SIGABRT);
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

void tr::quit(int32 error_code)
{
	tr::free();
	exit(error_code);
}
