/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/macros.h
 * All our evil macros go here (except for TR_TRY)
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

#ifndef _TRIPPIN_MACROS_H
#define _TRIPPIN_MACROS_H

namespace tr {

// evil macro fuckery
#define _TR_CONCAT2(A, B) A##B
#define _TR_CONCAT(A, B) _TR_CONCAT2(A, B)
#define TR_UNIQUE_NAME(Base) _TR_CONCAT(Base, __LINE__)

// defer
// usage: e.g. TR_DEFER(free(ptr));
template<typename Fn>
struct _defer
{
	Fn fn;

	_defer(Fn fn)
		: fn(fn)
	{
	}

	~_defer()
	{
		fn();
	}
};

template<typename Fn>
_defer<Fn> _defer_func(Fn fn)
{
	return _defer<Fn>{fn};
}

#define TR_DEFER(...)                                                              \
	[[maybe_unused]]                                                           \
	auto TR_UNIQUE_NAME(_tr_defer) = ::tr::_defer_func([&]() { __VA_ARGS__; })

} // namespace tr

#endif
