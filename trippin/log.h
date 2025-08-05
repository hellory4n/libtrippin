/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/log.h
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

#ifndef _TRIPPIN_LOG_H
#define _TRIPPIN_LOG_H

#include "trippin/common.h"

namespace tr {

namespace ConsoleColor {

// TODO colored output doesn't work on windows and i can't be bothered to fix it
#ifndef _WIN32
constexpr const char* RESET = "\033[0m";
constexpr const char* INFO = "\033[0;90m";
constexpr const char* WARN = "\033[0;93m";
constexpr const char* ERROR = "\033[0;91m";
#else
constexpr const char* RESET = "";
constexpr const char* INFO = "";
constexpr const char* WARN = "";
constexpr const char* ERROR = "";
#endif

} // namespace ConsoleColor

// Sets the log file to somewhere. There can be multiple log files.
void use_log_file(const char* path);

// it's annoying me over %li and %zu like the shut the fuck up i swear to fucking god
#if defined(TR_GCC_OR_CLANG) && !defined(TR_ONLY_MINGW_GCC)
[[gnu::format(printf, 1, 2)]]
#endif
// Log.
void log(const char* fmt, ...);

#if defined(TR_GCC_OR_CLANG) && !defined(TR_ONLY_MINGW_GCC)
[[gnu::format(printf, 1, 2)]]
#endif
// Log. (gray edition) (this is for libraries that use libtrippin so you can filter out library
// logs)
void info(const char* fmt, ...);

#if defined(TR_GCC_OR_CLANG) && !defined(TR_ONLY_MINGW_GCC)
[[gnu::format(printf, 1, 2)]]
#endif
// Oh nose.
void warn(const char* fmt, ...);

#if defined(TR_GCC_OR_CLANG) && !defined(TR_ONLY_MINGW_GCC)
[[gnu::format(printf, 1, 2)]]
#endif
// Oh god oh fuck. Note this doesn't crash and die everything, `tr::panic` does.
void error(const char* fmt, ...);

#if defined(TR_GCC_OR_CLANG) && !defined(TR_ONLY_MINGW_GCC)
[[gnu::format(printf, 1, 2)]]
#endif
// Oh god oh fuck. Note this crashes and kills everything, `tr::error` doesn't.
[[noreturn]]
void panic(const char* fmt, ...);

#if defined(TR_GCC_OR_CLANG) && !defined(TR_ONLY_MINGW_GCC)
[[gnu::format(printf, 2, 3)]]
#endif
void _impl_assert(const char* expr, const char* fmt, ...);

// Formatted assert?????????
#define TR_ASSERT_MSG(X, ...)                                                                      \
	if (!(X)) {                                                                                \
		tr::_impl_assert(#X, __VA_ARGS__);                                                 \
	}

#define TR_ASSERT(X)                                                                               \
	if (!(X)) {                                                                                \
		/* e.g. "failed assert 'false': aborting"*/                                        \
		tr::_impl_assert(#X, "aborting");                                                  \
	}

} // namespace tr

#endif
