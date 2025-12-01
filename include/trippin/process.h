/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/process.h
 * The life of a program: start, have children, panic, and
 * then get killed.
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

#ifndef _TRIPPIN_PROCESS_H
#define _TRIPPIN_PROCESS_H

#include "trippin/typedef.h"

namespace tr {

[[noreturn]]
void quit(int32 status);

// Placeholder panic function until I make better formatting shit
[[noreturn]]
void panicf(const char* fmt, ...);

// Marks somewhere as unreachable so that the compiler stops whining
[[noreturn]]
void unreachable(const char* ctx = "unknown");

// TODO write a description for this function not funny
[[noreturn]]
void todo(const char* ctx = "unknown");

} // namespace tr

#endif
