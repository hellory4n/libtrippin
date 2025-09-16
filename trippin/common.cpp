/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/common.cpp
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

#include "trippin/common.h"

// i'm using it dumbass
#include <cstdio> // IWYU pragma: keep
#include <cstdlib>

#include "trippin/collection.h"
#include "trippin/iofs.h"
#include "trippin/log.h"
#include "trippin/memory.h"

// they have to live somewhere
namespace tr {

Arena core_arena;
Array<File&> logfiles{core_arena};

File std_in;
File std_out;
File std_err;

// yes it's spelled that on purpose
Signal<bool> the_new_all_on_quit{core_arena};
bool panicking = false;
// so it doesn't keep emitting the signal forever
bool panicked_on_quit = false;

}

void tr::init()
{
	// man.
	tr::std_in = tr::core_arena.make_ref<File>();
	tr::std_in.fptr = stdin;
	tr::std_in.length = -1;
	tr::std_in.is_std = true;
	tr::std_in.mode = FileMode::READ_TEXT;

	tr::std_out = tr::core_arena.make_ref<File>();
	tr::std_out.fptr = stdout;
	tr::std_out.length = -1;
	tr::std_out.is_std = true;
	tr::std_out.mode = FileMode::WRITE_TEXT;

	tr::std_err = tr::core_arena.make_ref<File>();
	tr::std_err.fptr = stderr;
	tr::std_err.length = -1;
	tr::std_err.is_std = true;
	tr::std_err.mode = FileMode::WRITE_TEXT;

	logfiles.add(tr::std_out);

	tr::_init_paths();

	tr::info("initialized libtrippin %s", tr::VERSION);
}

void tr::free()
{
	// so it doesn't keep emitting the signal forever
	if (tr::panicked_on_quit) {
		tr::warn("panicked from tr::call_on_quit, aborting");
		return;
	}

	// c++ is fucking with the template varargs
	// idk why i can't just pass the variable
	// but it works by negating it twice (which makes it the same value)
	// TODO rewrite in rust
	tr::the_new_all_on_quit.emit(!!tr::panicking);
	tr::info("deinitialized libtrippin");
}

[[noreturn]]
void tr::quit(int32 error_code)
{
	tr::free();
	exit(error_code);
}

void tr::call_on_quit(const std::function<void(bool is_panic)>& func)
{
	tr::the_new_all_on_quit.connect(func);
}
