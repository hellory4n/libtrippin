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

#include <type_traits>
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

#define TR_DEFER(...)                                                               \
	[[maybe_unused]]                                                            \
	auto _TR_UNIQUE_NAME(_tr_defer) = ::tr::_defer_func([&]() { __VA_ARGS__; })

// Shorthand for calling a function, unwrapping if valid, and returning an error otherwise
// example: TR_TRY_ASSIGN(int32 var, some_function());
#ifdef __GNUC__
	#define TR_TRY_ASSIGN(Var, ...)                                   \
		/* clang-format off */ \
		_Pragma("GCC warning \"TR_TRY_ASSIGN is deprecated: you can now use the all-new new all TR_TRY, e.g. int x = TR_TRY(whatever)\"");                                  \
		/* clang-format on */                                     \
		const auto _TR_UNIQUE_NAME(_tr_try_tmp) = (__VA_ARGS__);  \
		if (_TR_UNIQUE_NAME(_tr_try_tmp).is_invalid()) {          \
			return _TR_UNIQUE_NAME(_tr_try_tmp).unwrap_err(); \
		}                                                         \
		Var = _TR_UNIQUE_NAME(_tr_try_tmp).unwrap()
#else
	#define TR_TRY_ASSIGN(Var, ...)                                   \
		const auto _TR_UNIQUE_NAME(_tr_try_tmp) = (__VA_ARGS__);  \
		if (_TR_UNIQUE_NAME(_tr_try_tmp).is_invalid()) {          \
			return _TR_UNIQUE_NAME(_tr_try_tmp).unwrap_err(); \
		}                                                         \
		Var = _TR_UNIQUE_NAME(_tr_try_tmp).unwrap()
#endif

// idk why would you need more than that for a mere error
constexpr unsigned MAX_ERROR_SIZE = 128;

// to support TR_TRY
[[maybe_unused]]
static thread_local bool _last_try_failed;
[[maybe_unused]]
static thread_local unsigned char _last_try_error[MAX_ERROR_SIZE];

template<typename T>
requires(!std::is_reference_v<T>)
union _evilTryUnion {
	T val;
	unsigned char no_val = 0;
};

// so Result<void> doesn't break everything
template<>
union _evilTryUnion<void> {
	unsigned char val;
	unsigned char no_val;
};

// our most evil macro to date
// example: int x = TR_TRY(function());
// note that TR_TRY only works with variables
// so this is unsupported: function(TR_TRY(other_function()))
#define TR_TRY(...)                                                                               \
	/* i know gcc statement expressions exist but unfortunately msvc is a thing */            \
	/* both gcc and clang can optimize this pretty well (msvc is stupid) */                   \
	/* yes i checked the assembly i'm insane */                                               \
	[&]() {                                                                                   \
		const auto _TR_UNIQUE_NAME(_tr_try) = (__VA_ARGS__);                              \
		using _TrTryType = decltype(_TR_UNIQUE_NAME(_tr_try))::Type;                      \
		if (_TR_UNIQUE_NAME(_tr_try).is_valid()) [[likely]] {                             \
			/* if constexpr only works properly on templates */                       \
			/* so use a template lambda which is a thing some reason */               \
			auto _TR_UNIQUE_NAME(_tr_man) = []<typename _TrTryType2>(                 \
								_TrTryType2 result                \
							) {                                       \
				if constexpr (::std::is_void_v<typename _TrTryType2::Type>) {     \
					return ::tr::_evilTryUnion<typename _TrTryType2::Type>{}; \
				}                                                                 \
				else {                                                            \
					return ::tr::_evilTryUnion<typename _TrTryType2::Type>{   \
						.val = result.unwrap()                            \
					};                                                        \
				}                                                                 \
			};                                                                        \
			return _TR_UNIQUE_NAME(_tr_man)(_TR_UNIQUE_NAME(_tr_try));                \
		}                                                                                 \
		else {                                                                            \
			::tr::_last_try_failed = true;                                            \
			/* errors are stored inside Result<T> in a buffer that's always the same  \
			 * size*/                                                                 \
			::std::memcpy(                                                            \
				::tr::_last_try_error,                                            \
				reinterpret_cast<const void*>(                                    \
					&_TR_UNIQUE_NAME(_tr_try).unwrap_err()                    \
				),                                                                \
				::tr::MAX_ERROR_SIZE                                              \
			);                                                                        \
			return ::tr::_evilTryUnion<_TrTryType>{};                                 \
		}                                                                                 \
	}()                                                                                       \
		.val; /* exploiting unions, if the value is invalid we're about to return so it   \
			   doesn't matter */                                                      \
	/* pure uncut macro abuse */                                                              \
	if (::tr::_last_try_failed) {                                                             \
		::tr::_last_try_failed = false;                                                   \
		return {*reinterpret_cast<const ::tr::Error*>(::tr::_last_try_error)};            \
	}

// Similar to `TR_ASSERT`, but instead of panicking, it returns an error.
// example: TR_TRY_ASSERT(false, tr::StringError("WHAÇÇT"));
#define TR_TRY_ASSERT(X, ...)         \
	if (!(X)) {                   \
		return (__VA_ARGS__); \
	}
}

#endif
