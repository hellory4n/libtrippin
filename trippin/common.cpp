/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/common.hpp
 * Numbers, macros, and utility structs
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
// i'm using it tho??????????
#include <stdio.h> // IWYU pragma: keep

#include "log.hpp"
#include "memory.hpp"
#include "iofs.hpp"
#include "collection.hpp"
#include "common.hpp"

// they have to live somewhere
namespace tr {
	Arena core_arena(tr::kb_to_bytes(128));

	MemoryInfo memory_info;
	Array<File*> logfiles(core_arena);

	File std_in;
	File std_out;
	File std_err;

	Signal<void> on_quit(core_arena);
}

void tr::init()
{
	// man.
	tr::std_in = tr::core_arena.make<File>();
	tr::std_in.fptr = stdin;
	tr::std_in.length = -1;
	tr::std_in.is_std = true;
	tr::std_in.mode = FileMode::READ_TEXT;

	tr::std_out = tr::core_arena.make<File>();
	tr::std_out.fptr = stdout;
	tr::std_out.length = -1;
	tr::std_out.is_std = true;
	tr::std_out.mode = FileMode::WRITE_TEXT;

	tr::std_err = tr::core_arena.make<File>();
	tr::std_err.fptr = stderr;
	tr::std_err.length = -1;
	tr::std_err.is_std = true;
	tr::std_err.mode = FileMode::WRITE_TEXT;

	logfiles.add(&tr::std_out);

	tr::info("initialized libtrippin %s", tr::VERSION);
}

void tr::free()
{
	// TODO what happens if a function panics here?
	tr::on_quit.emit();
	tr::info("deinitialized libtrippin");
}

void tr::quit(int32 error_code)
{
	tr::free();
	exit(error_code);
}

void tr::call_on_quit(std::function<void(void)> func)
{
	tr::on_quit.connect(func);
}
