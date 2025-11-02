/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/bits/macros.h
 * All our evil macros go here
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

#ifndef _TRIPPIN_BITS_MACROS_H
#define _TRIPPIN_BITS_MACROS_H

#ifndef _TRIPPIN_COMMON_H
	#error "Never include trippin/bits/macros.h directly. Use trippin/common.h."
#endif

namespace tr {

// evil macro fuckery
#define _TR_CONCAT2(A, B) A##B
#define _TR_CONCAT(A, B) _TR_CONCAT2(A, B)
#define _TR_UNIQUE_NAME(Base) _TR_CONCAT(Base, __LINE__)

// Formatted assert???? implemented in log.cpp because yes
[[noreturn]]
void _impl_assert(const char* expr, const char* fmt, ...);

// Formatted assert?????????
#define TR_ASSERT_MSG(X, ...)                      \
	if (!(X)) {                                \
		tr::_impl_assert(#X, __VA_ARGS__); \
	}

#define TR_ASSERT(...)                                      \
	if (!(__VA_ARGS__)) {                               \
		/* e.g. "failed assert 'false': aborting"*/ \
		tr::_impl_assert(#__VA_ARGS__, "aborting"); \
	}

// yeah
#define TR_UNREACHABLE()                                                                           \
	tr::panic(                                                                                 \
		"unreachable code at %s:%i has been reached. for end users, this is a bug in the " \
		"program. please contact the developers.",                                         \
		__FILE__, __LINE__                                                                 \
	)

#define TR_TODO()                                                                            \
	tr::panic(                                                                           \
		"unimplemented functionality at %s:%i. for end users, this is a bug in the " \
		"program. please contact the developers.",                                   \
		__FILE__, __LINE__                                                           \
	)

// defer
// usage: e.g. TR_DEFER(free(ptr));
template<typename Fn>
struct Defer
{
	Fn fn;

	Defer(Fn fn)
		: fn(fn)
	{
	}

	~Defer()
	{
		fn();
	}
};

template<typename Fn>
Defer<Fn> _defer_func(Fn fn)
{
	return Defer<Fn>(fn);
}

#define TR_DEFER(...)                                                             \
	[[maybe_unused]]                                                          \
	auto _TR_UNIQUE_NAME(_tr_defer) = tr::_defer_func([&]() { __VA_ARGS__; })

// Shorthand for calling a function, unwrapping if valid, and returning an error otherwise
// example: TR_TRY_ASSIGN(int32 var, some_function());
#define TR_TRY_ASSIGN(Var, ...)                                   \
	const auto _TR_UNIQUE_NAME(_tr_try_tmp) = (__VA_ARGS__);  \
	if (_TR_UNIQUE_NAME(_tr_try_tmp).is_invalid()) {          \
		return _TR_UNIQUE_NAME(_tr_try_tmp).unwrap_err(); \
	}                                                         \
	Var = _TR_UNIQUE_NAME(_tr_try_tmp).unwrap()

// `TR_TRY_ASSIGN` but for `tr::Result<void, E>`, or for when you don't care about the result
// example: TR_TRY(some_function());
#define TR_TRY(...)                                               \
	const auto _TR_UNIQUE_NAME(_tr_try_tmp) = (__VA_ARGS__);  \
	if (_TR_UNIQUE_NAME(_tr_try_tmp).is_invalid()) {          \
		return _TR_UNIQUE_NAME(_tr_try_tmp).unwrap_err(); \
	}

// Similar to `tr::assert`, but instead of panicking, it returns an error.
// example: TR_TRY_ASSERT(false, tr::StringError("something went wrong"));
#define TR_TRY_ASSERT(X, ...)         \
	if (!(X)) {                   \
		return (__VA_ARGS__); \
	}

}

#endif
