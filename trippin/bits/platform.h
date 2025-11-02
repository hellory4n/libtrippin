/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/bits/platform.h
 * Platform/compiler-specific macros
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

#ifndef _TRIPPIN_BITS_PLATFORM_H
#define _TRIPPIN_BITS_PLATFORM_H

#ifndef _TRIPPIN_COMMON_H
	#error "Never include trippin/bits/platform.h directly. Use trippin/common.h."
#endif

// get compiler
// checking between gcc and clang is useful because some warnings are different
#if defined(__clang__)
	#define TR_ONLY_CLANG
#elif defined(__GNUC__)
	#define TR_ONLY_GCC
#endif

// but they're similar enough that we can usually check for both
#ifdef __GNUC__
	#define TR_GCC_OR_CLANG
#endif

#ifdef _MSC_VER
	#define TR_ONLY_MSVC
#endif

// slightly esoteric compiler variants
#if defined(__GNUC__) && defined(_WIN32)
	#define TR_ONLY_MINGW_GCC
#endif
#if defined(_MSC_VER) && defined(__clang__)
	#define TR_ONLY_CLANG_CL
#endif
#if defined(__clang__) && defined(__apple_build_version__)
	#define TR_ONLY_APPLE_CLANG
#endif

// os
#ifdef __unix__
	#define TR_OS_UNIXLIKE
#endif

#if defined(__linux__)
	#define TR_OS_LINUX
#endif

#if defined(__APPLE__) && defined(__MACH__) && !defined(__IOS__)
	#define TR_OS_MACOSX
	#define TR_OS_DARWIN
	#define TR_OS_BSDLIKE
#endif

#if defined(__IOS__)
	#define TR_OS_IOS
	#define TR_OS_DARWIN
	#define TR_OS_BSDLIKE
#endif

#if defined(__FreeBSD__)
	#define TR_OS_BSDLIKE
	#define TR_OS_FREEBSD
#endif

#if defined(__NetBSD__)
	#define TR_OS_BSDLIKE
	#define TR_OS_NETBSD
#endif

#if defined(__OpenBSD__)
	#define TR_OS_BSDLIKE
	#define TR_OS_OPENBSD
#endif

#if defined(__DragonFly__)
	#define TR_OS_BSDLIKE
	#define TR_OS_DRAGONFLY
#endif

#if defined(_WIN32) || defined(_WIN64)
	#define TR_OS_WINDOWS
#endif

#if defined(__ANDROID__)
	#define TR_OS_ANDROID
#endif

#if defined(__EMSCRIPTEN__)
	#define TR_OS_EMSCRIPTEN
#endif

// architecture
#if defined(__amd64__) || defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64)
	#define TR_ARCH_X86_64
	#define TR_ARCH_X86
	#define TR_ARCH_64_BITS
#elif defined(__i386__) || defined(__i386) || defined(_M_IX86) || defined(_X86_)
	#define TR_ARCH_X86_32
	#define TR_ARCH_X86
	#define TR_ARCH_32_BITS
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
	#define TR_ARCH_ARM64
	#define TR_ARCH_ARM
	#define TR_ARCH_64_BITS
#elif defined(__arm__) || defined(__arm) || defined(_ARM) || defined(_M_ARM)
	#define TR_ARCH_ARM32
	#define TR_ARCH_ARM
	#define TR_ARCH_32_BITS
#endif

#ifdef __riscv
	#define TR_ARCH_RISCV
	#ifdef __riscv_xlen
		#if __riscv_xlen == 32
			#define TR_ARCH_RISCV_32
			#define TR_ARCH_32_BITS
		#elif __riscv_xlen == 64
			#define TR_ARCH_RISCV_32
			#define TR_ARCH_32_BITS
		#endif
	#endif
#endif

// weirdly msvc only defines __cplusplus as c++98, that is unless you either set a
// compile flag, or, use another macro, which isn't defined in all versions (so
// we'll assume if it's not defined, it's too old for c++20 anyway)
#ifndef TR_ONLY_MSVC
	#define TR_CPLUSPLUS __cplusplus
#else
	#ifdef _MSVC_LANG
		#define TR_CPLUSPLUS _MSVC_LANG
	#else
		#error "Your Visual Studio installation is too old, please update to Visual Studio 2022 or higher"
	#endif
#endif

// check for c++20 support
#if TR_CPLUSPLUS < 202002L
	#error "libtrippin requires C++20 or higher"
#endif

// if only they made __has_feature(__has_feature)
#ifndef __has_feature
	#define __has_feature(...) false
#endif

// silence warnings
// TODO msvc version
#ifdef TR_GCC_OR_CLANG
	#define TR_GCC_PRAGMA(X) _Pragma(#X)

	/* so you don't have to check between GCC and clang, it'll just shut up */
	#ifdef TR_ONLY_CLANG
		#define TR_GCC_IGNORE_WARNING(Warning) \
			TR_GCC_PRAGMA(GCC diagnostic push)                  \
			TR_GCC_PRAGMA(GCC diagnostic ignored "-Wunknown-warning-option")                  \
			TR_GCC_PRAGMA(GCC diagnostic push)                  \
			TR_GCC_PRAGMA(GCC diagnostic ignored #Warning)
	#else
		#define TR_GCC_IGNORE_WARNING(Warning) \
			TR_GCC_PRAGMA(GCC diagnostic push)                  \
			TR_GCC_PRAGMA(GCC diagnostic ignored "-Wpragmas")                  \
			TR_GCC_PRAGMA(GCC diagnostic push)                  \
			TR_GCC_PRAGMA(GCC diagnostic ignored #Warning)
	#endif

	#define TR_GCC_RESTORE() \
		TR_GCC_PRAGMA(GCC diagnostic pop)    \
		TR_GCC_PRAGMA(GCC diagnostic pop)
#else
	#define TR_GCC_PRAGMA(X)
	#define TR_GCC_IGNORE_WARNING(Warning)
	#define TR_GCC_RESTORE()
#endif

// asan stuff
#if (__has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)) && \
	__has_include(<sanitizer/asan_interface.h>)
	#include <sanitizer/asan_interface.h>
	#define TR_HAS_ADDRESS_SANITIZER
	#define TR_ASAN_POISON_MEMORY(addr, size) __asan_poison_memory_region((addr), (size))
	#define TR_ASAN_UNPOISON_MEMORY(addr, size) __asan_unpoison_memory_region((addr), (size))
	#define TR_LSAN_REGISTER_ROOT_REGION(base, size) __lsan_register_root_region((base), (size))
	#define TR_LSAN_UNREGISTER_ROOT_REGION(base, size)    \
		__lsan_unregister_root_region((base), (size))
#else
	#define TR_ASAN_POISON_MEMORY(addr, size)
	#define TR_ASAN_UNPOISON_MEMORY(addr, size)
	#define TR_LSAN_REGISTER_ROOT_REGION(base, size)
	#define TR_LSAN_UNREGISTER_ROOT_REGION(base, size)
#endif

// used for detecting dangling references and pointers and stuff
// e.g. void* alloc_memory(Arena& arena TR_LIFETIMEBOUND)
// or void* Arena::alloc() TR_LIFETIMEBOUND
#ifdef TR_ONLY_CLANG
	#define TR_LIFETIMEBOUND [[clang::lifetimebound]]
#elif defined(TR_ONLY_MSVC)
	#define TR_LIFETIMEBOUND [[msvc::lifetimebound]]
	// we have to enable the warning so it actually does something
	// https://learn.microsoft.com/en-us/cpp/code-quality/c26815
	#pragma warning(default : 26815)
#else
	#define TR_LIFETIMEBOUND
#endif

#endif
