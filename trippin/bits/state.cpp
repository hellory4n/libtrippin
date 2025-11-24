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

#include <atomic>
#include <memory>

#include "trippin/error.h"
#include "trippin/iofs.h"
#include "trippin/memory.h"
#include "trippin/string.h"
#include "trippin/util.h"
// man
/* clang-format off */
#undef _TRIPPIN_BITS_STATE_H
#define _TR_BULLSHIT_SO_THAT_IT_WORKS
#include "trippin/bits/state.h"
/* clang-format on */

// TODO this has to be thread-safe somehow
// you could use a thread_local variable but freeing becomes fucky and some state has to be shared
// so i'm just using an std::atomic<std::shared_ptr<T>> here, along with some helper functions so
// that it gets freed properly
// the std::shared_ptr is because otherwise it doesn't work on classes with vtables, also i can't
// get a reference out of std::atomic without that?
// there's probably a better way but this is good enough for now

void free_arena(tr::Arena* arena)
{
	arena->free();
	delete arena;
}

tr::Arena& tr::_tr::core_arena()
{
	static bool initialized = false;
	// THE HORROR HAS BECOME TOO GREAT FOR ANY ONE MIND TO BARE
	static std::atomic<std::shared_ptr<Arena>> core_arena;
	if (!initialized) {
		core_arena = std::shared_ptr<Arena>{new Arena(), free_arena};
		initialized = true;
	}
	return *static_cast<std::shared_ptr<Arena>>(core_arena);
}

tr::Arena& tr::_tr::consty_arena()
{
	static bool initialized = false;
	// THE HORROR HAS BECOME TOO GREAT FOR ANY ONE MIND TO BARE
	static std::atomic<std::shared_ptr<Arena>> consty_arena;
	if (!initialized) {
		consty_arena = std::shared_ptr<Arena>{new Arena(), free_arena};
		initialized = true;
	}
	return *static_cast<std::shared_ptr<Arena>>(consty_arena);
}

tr::Array<tr::File>& tr::_tr::logfiles()
{
	static bool initialized = false;
	// THE HORROR HAS BECOME TOO GREAT FOR ANY ONE MIND TO BARE
	static std::atomic<std::shared_ptr<Array<File>>> logfiles;
	if (!initialized) {
		logfiles = std::shared_ptr<Array<File>>{new Array<File>{core_arena()}};
		initialized = true;
	}
	return *static_cast<std::shared_ptr<Array<File>>>(logfiles);
}

tr::Signal<bool>& tr::_tr::on_quit()
{
	static bool initialized = false;
	// THE HORROR HAS BECOME TOO GREAT FOR ANY ONE MIND TO BARE
	static Signal<bool> on_quit;
	if (!initialized) {
		on_quit = Signal<bool>{tr::_tr::core_arena()};
		initialized = true;
	}
	return on_quit;
}

tr::WrapArena& tr::_tr::tmp_strings()
{
	static bool initialized = false;
	// THE HORROR HAS BECOME TOO GREAT FOR ANY ONE MIND TO BARE
	static std::atomic<std::shared_ptr<WrapArena>> arena;
	if (!initialized) {
		arena = std::shared_ptr<WrapArena>{new WrapArena(MAX_TEMP_STRING_SIZE), free_arena};
		initialized = true;
	}
	return *static_cast<std::shared_ptr<WrapArena>>(arena);
}

tr::HashMap<tr::ErrorType, tr::String (*)(tr::ErrorArgs args)>& tr::_tr::error_table()
{
	static bool initialized = false;
	// THE HORROR HAS BECOME TOO GREAT FOR ANY ONE MIND TO BARE
	static std::atomic<std::shared_ptr<HashMap<ErrorType, String (*)(ErrorArgs args)>>>
		error_table;
	if (!initialized) {
		// AND YOU MAY FIND YOURSELF
		error_table = std::shared_ptr<HashMap<ErrorType, String (*)(ErrorArgs)>>{
			new HashMap<ErrorType, String (*)(ErrorArgs args)>(tr::_tr::core_arena())
		};
		initialized = true;
	}
	return *static_cast<std::shared_ptr<HashMap<ErrorType, String (*)(ErrorArgs args)>>>(
		error_table
	);
}
