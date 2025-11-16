/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/bits/state.cpp
 * Also manages libtrippin's global state. Amazing.
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

/* clang-format off */
#include "trippin/memory.h"
#include "trippin/iofs.h"
#include "trippin/util.h"
#include "trippin/error.h"
// man
#undef _TRIPPIN_BITS_STATE_H
#define _TR_BULLSHIT_SO_THAT_IT_WORKS
#include "trippin/bits/state.h"
/* clang-format on */

tr::Arena& tr::_tr::core_arena()
{
	static bool initialized = false;
	static Arena core_arena;
	if (!initialized) {
		core_arena = Arena{};
		initialized = true;
	}
	return core_arena;
}

tr::Arena& tr::_tr::consty_arena()
{
	static bool initialized = false;
	static Arena consty_arena;
	if (!initialized) {
		consty_arena = Arena{};
		initialized = true;
	}
	return consty_arena;
}

tr::Array<tr::File>& tr::_tr::logfiles()
{
	static bool initialized = false;
	static Array<File> logfiles;
	if (!initialized) {
		logfiles = Array<File>{tr::_tr::core_arena()};
		initialized = true;
	}
	return logfiles;
}

tr::Signal<bool>& tr::_tr::on_quit()
{
	static bool initialized = false;
	static Signal<bool> on_quit;
	if (!initialized) {
		on_quit = Signal<bool>{tr::_tr::core_arena()};
		initialized = true;
	}
	return on_quit;
}

tr::HashMap<tr::ErrorType, tr::String (*)(tr::ErrorArgs args)>& tr::_tr::error_table()
{
	static bool initialized = false;
	static HashMap<ErrorType, String (*)(ErrorArgs args)> error_table;
	if (!initialized) {
		error_table = HashMap<ErrorType, String (*)(ErrorArgs)>{tr::_tr::core_arena()};
		initialized = true;
	}
	return error_table;
}
