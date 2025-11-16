/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/bits/state.h
 * Manages libtrippin's global state
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

#ifndef _TRIPPIN_BITS_STATE_H
#define _TRIPPIN_BITS_STATE_H

namespace tr {

// TODO there's no way this is thread safe

namespace _tr {
	// functions instead of global variables so that we can control when they're initialized,
	// which is useful so that the error system and its 7634657 static vars don't blow up
	// everything with a race condition

	// this isn't necessary for primitives, which get initialized first and can just use a
	// global variable
	// (https://en.cppreference.com/w/cpp/language/initialization.html#Static_initialization)

// unfortunately some headers need this so we have to use this bullshit
#ifdef _TRIPPIN_MEMORY_H
	Arena& core_arena();

	// std::initializer_list<T> doesn't live very long. to prevent fucking (dangling ptrs), we
	// have to copy the data somewhere that lasts longer, and trap them in purgatory for as long
	// as the program is open.
	Arena& consty_arena();

	#if defined(_TRIPPIN_IOFS_H) && defined(_TR_BULLSHIT_SO_THAT_IT_WORKS)
	Array<File>& logfiles();
	#endif
#endif

#if defined(_TRIPPIN_UTIL_H) && defined(_TR_BULLSHIT_SO_THAT_IT_WORKS)
	Signal<bool>& on_quit();

	#ifdef _TRIPPIN_ERROR_H
	HashMap<ErrorType, String (*)(ErrorArgs args)>& error_table();
	#endif
#endif

} // namespace _tr

} // namespace tr

#endif
