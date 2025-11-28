/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/platform.h
 * Platform info + compiler extensions
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

#ifndef _TRIPPIN_PLATFORM_H
#define _TRIPPIN_PLATFORM_H

// get compiler
// checking between gcc and clang is useful because some warnings are different
#if defined(__clang__)
	#define TR_COMPILER_CLANG
#elif defined(__GNUC__)
	#define TR_COMPILER_GCC
#endif

// but they're similar enough that we can usually check for both
#ifdef __GNUC__
	#define TR_COMPILER_GCCLIKE
#endif

#ifdef _MSC_VER
	#define TR_COMPILER_MSVC
#endif

// slightly esoteric compiler variants
#if defined(__GNUC__) && (defined(__MINGW32__) || defined(__MINGW64__))
	#define TR_COMPILER_MINGW_GCC
#endif
#if defined(_MSC_VER) && defined(__clang__)
	#define TR_COMPILER_CLANG_CL
#endif
#if defined(__clang__) && defined(__apple_build_version__)
	#define TR_COMPILER_APPLE_CLANG
#endif

// disable msvc's stupid errors with libc functions
#ifndef _CRT_SECURE_NO_WARNINGS
	#define _CRT_SECURE_NO_WARNINGS
#endif

// os
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

#if defined(__unix__) || defined(TR_OS_LINUX) || defined(TR_OS_BSDLIKE)
	#define TR_OS_UNIXLIKE
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
// only x86, arm, and maybe risc-v are really used nowadays so
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

// weirdly msvc only defines __cplusplus as c++98, that is unless you either set a compiler
// flag, or, use another macro, which isn't defined in all versions (so we'll assume if it's
// not defined, it's too old for c++20 anyway)
#ifndef TR_COMPILER_MSVC
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
#ifdef TR_COMPILER_GCCLIKE
	#define TR_GCC_PRAGMA(X) _Pragma(X)

	/* so you don't have to check between GCC and clang, it'll just shut up */
	#ifdef TR_COMPILER_CLANG
		#define TR_GCC_IGNORE_WARNING(Warning) \
			TR_GCC_PRAGMA(GCC diagnostic push)                  \
			TR_GCC_PRAGMA(GCC diagnostic ignored "-Wunknown-warning-option")                  \
			TR_GCC_PRAGMA(GCC diagnostic push)                  \
			TR_GCC_PRAGMA(GCC diagnostic ignored Warning)
	#else
		#define TR_GCC_IGNORE_WARNING(Warning) \
			TR_GCC_PRAGMA(GCC diagnostic push)                  \
			TR_GCC_PRAGMA(GCC diagnostic ignored "-Wpragmas")                  \
			TR_GCC_PRAGMA(GCC diagnostic push)                  \
			TR_GCC_PRAGMA(GCC diagnostic ignored Warning)
	#endif

	#define TR_GCC_RESTORE() \
		TR_GCC_PRAGMA(GCC diagnostic pop)    \
		TR_GCC_PRAGMA(GCC diagnostic pop)
#else
	#define TR_GCC_PRAGMA(X)
	#define TR_GCC_IGNORE_WARNING(Warning)
	#define TR_GCC_RESTORE()
#endif

// always inline
#ifdef TR_COMPILER_MSVC
	#define TR_ALWAYS_INLINE __forceinline
#elif defined(TR_COMPILER_GCCLIKE)
	#define TR_ALWAYS_INLINE [[gnu::always_inline]] inline
#else
	#define TR_ALWAYS_INLINE inline
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

#endif
