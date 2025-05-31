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

void init()
{
	tr::liblog("initialized libtrippin %s", tr::VERSION);
}

void free()
{
	tr::liblog("deinitialized libtrippin");
	fclose(tr::logfile);
}

void use_log_file(const char* path)
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
	struct tm* tm_info = localtime(&now);
	strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", tm_info);

	// TODO maybe increase in the future?
	char buf[256];
	vsnprintf(buf, sizeof(buf), fmt, arg);

	if (tr::logfile == nullptr) {
		printf(
			"%s [%s] no log file available. did you forget to call tr::init()?%s\n",
			color, timestr, ConsoleColor::RESET
		);
	}
	else {
		fprintf(tr::logfile, "[%s] %s%s\n", timestr, prefix, buf);
		fflush(tr::logfile);
	}

	printf("%s[%s] %s%s%s\n", color, timestr, prefix, buf, ConsoleColor::RESET);
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

TR_LOG_FUNC(1, 2) void log(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	__log("", "", false, fmt, args);
	va_end(args);
}

TR_LOG_FUNC(1, 2) void liblog(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	__log(ConsoleColor::LIB_INFO, "", false, fmt, args);
	va_end(args);
}

TR_LOG_FUNC(1, 2) void warn(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	__log(ConsoleColor::WARN, "", false, fmt, args);
	va_end(args);
}

TR_LOG_FUNC(1, 2) void error(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	__log(ConsoleColor::ERROR, "", false, fmt, args);
	va_end(args);
}

TR_LOG_FUNC(1, 2) void panic(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	__log(ConsoleColor::ERROR, "panic: ", true, fmt, args);
	// Function declared 'noreturn' should not return
	// (this will never happen because __log panics first)
	exit(1);
	va_end(args);
}

TR_LOG_FUNC(2, 3) void assert(bool x, const char* fmt, ...)
{
	#ifdef DEBUG
	if (!x) {
		va_list args;
		va_start(args, fmt);
		__log(ConsoleColor::ERROR, "failed assert: ", true, fmt, args);
		va_end(args);
	}
	#else
	// the compiler is complaining about unused arguments
	(void)x;
	(void)fmt;
	#endif
}

}
