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

#ifndef _TRIPPIN_H
#define _TRIPPIN_H
#include <cstdint>
#include <cstddef>
#include <cstdlib>

#if (defined(__GNUC__) || defined(__clang__)) && !defined(_WIN32)
// counting starts at 1 lmao
#define TR_LOG_FUNC(fmt_idx, varargs_idx) __attribute__((format(printf, fmt_idx, varargs_idx)))
#else
#define TR_LOG_FUNC(fmt_idx, varargs_idx)
#endif

namespace tr {

// I sure love versions.
constexpr const char* VERSION = "v2.0.0";

namespace ConsoleColor {
	// TODO colored output doesn't work on windows and i can't be bothered to fix it
	#ifndef _WIN32
	constexpr const char* RESET    = "\033[0m";
	constexpr const char* LIB_INFO = "\033[0;90m";
	constexpr const char* WARN     = "\033[0;93m";
	constexpr const char* ERROR    = "\033[0;91m";
	#else
	constexpr const char* RESET    = "";
	constexpr const char* LIB_INFO = "";
	constexpr const char* WARN     = "";
	constexpr const char* ERROR    = "";
	#endif
}

// Initializes the bloody library lmao.
void init(const char* logfile);

// Deinitializes the bloody library lmao.
void free();

// Log.
TR_LOG_FUNC(1, 2) void log(const char* fmt, ...);

// Log. (gray edition) (this is for libraries that use libtrippin so you can filter out library logs)
TR_LOG_FUNC(1, 2) void liblog(const char* fmt, ...);

// Oh nose.
TR_LOG_FUNC(1, 2) void warn(const char* fmt, ...);

// Oh god oh fuck. Note this doesn't crash and die everything, `tr::panic` does.
TR_LOG_FUNC(1, 2) void error(const char* fmt, ...);

// Oh god oh fuck. Note this crashes and kills everything, `tr::error` doesn't.
TR_LOG_FUNC(1, 2) void panic(const char* fmt, ...);

// Formatted assert?????????
TR_LOG_FUNC(2, 3) void assert(bool x, const char* fmt, ...);

}

#endif
