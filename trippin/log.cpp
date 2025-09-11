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

#include <cstdarg>
#include <cstdlib>
#include <ctime>
#ifdef _MSC_VER
	#include <intrin.h>
#endif

#include "trippin/collection.h"
#include "trippin/common.h"
#include "trippin/iofs.h"
#include "trippin/log.h"
#include "trippin/memory.h"

namespace tr {

extern Arena core_arena;
extern Array<File&> logfiles;
extern Signal<bool> the_new_all_on_quit;
extern bool panicking;
extern bool panicked_on_quit;

void _log(const char* color, const char* prefix, bool panic, const char* fmt, va_list arg);

}

void tr::use_log_file(const char* path)
{
	Result<File&, const Error&> f = File::open(core_arena, path, FileMode::WRITE_TEXT);
	if (!f.is_valid()) {
		tr::warn("couldn't use log file '%s': %s", path, f.unwrap_err().message().buf());
	}
	File& file = f.unwrap();
	logfiles.add(file);

	tr::info("using log file \"%s\"", path);
}

void tr::_log(const char* color, const char* prefix, bool panic, const char* fmt, va_list arg)
{
	// you understand mechanical hands are the ruler of everything (ah)
	// TODO tr::time?? idfk
	char timestr[32];
	time_t now = time(nullptr);
// FUCK ME
#ifdef _WIN32
	struct tm tm_info;
	localtime_s(&tm_info, &now);
	strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", &tm_info);
#else
	struct tm* tm_info = localtime(&now);
	strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", tm_info);
#endif

	va_list argmaballs;
	va_copy(argmaballs, arg);
	String buf = tr::fmt_args(tr::scratchpad(), fmt, argmaballs);
	va_end(argmaballs);

	for (auto [_, file] : tr::logfiles) {
		if (file.is_std) {
			file.write_string(color);
		}
		file.printf("[%s] %s%s", timestr, prefix, buf.buf());
		if (file.is_std) {
			file.write_string(ConsoleColor::RESET);
		}
		file.write_string("\n");

		file.flush();
	}

	if (panic) {
		// i'm having a minor aneurysm rn
		if (tr::panicking) {
			tr::panicked_on_quit = true;
		}
		tr::panicking = true;
		tr::free();

// i don't think anyone is gonna be compiling with intel c++ or whatever the fuck
// but it should still work anyway
#ifdef TR_ONLY_MSVC
		__debugbreak();
#elif defined(TR_GCC_OR_CLANG)
		__builtin_trap();
#else
		abort();
#endif
	}
}

#if defined(TR_GCC_OR_CLANG) && !defined(TR_ONLY_MINGW_GCC)
[[gnu::format(printf, 1, 2)]]
#endif
void tr::log(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	_log("", "", false, fmt, args);
	va_end(args);
}

#if defined(TR_GCC_OR_CLANG) && !defined(TR_ONLY_MINGW_GCC)
[[gnu::format(printf, 1, 2)]]
#endif
void tr::info(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	_log(tr::ConsoleColor::INFO, "", false, fmt, args);
	va_end(args);
}

#if defined(TR_GCC_OR_CLANG) && !defined(TR_ONLY_MINGW_GCC)
[[gnu::format(printf, 1, 2)]]
#endif
void tr::warn(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	_log(tr::ConsoleColor::WARN, "warning: ", false, fmt, args);
	va_end(args);
}

#if defined(TR_GCC_OR_CLANG) && !defined(TR_ONLY_MINGW_GCC)
[[gnu::format(printf, 1, 2)]]
#endif
void tr::error(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	_log(tr::ConsoleColor::ERROR, "error: ", false, fmt, args);
	va_end(args);
}

#if defined(TR_GCC_OR_CLANG) && !defined(TR_ONLY_MINGW_GCC)
[[gnu::format(printf, 1, 2)]]
#endif
[[noreturn]]
void tr::panic(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	_log(tr::ConsoleColor::ERROR, "panic: ", true, fmt, args);
	// Function declared 'noreturn' should not return
	// (this will never happen because __log panics first)
	exit(1);
	va_end(args);
}

#if defined(TR_GCC_OR_CLANG) && !defined(TR_ONLY_MINGW_GCC)
[[gnu::format(printf, 2, 3)]]
#endif
[[noreturn]]
void tr::_impl_assert(const char* expr, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	String prefix = tr::fmt(tr::scratchpad(), "failed assert \"%s\": ", expr);

	_log(tr::ConsoleColor::ERROR, prefix, true, fmt, args);
	va_end(args);
	// straight up [[noreturn]]ing it
	exit(1);
}
