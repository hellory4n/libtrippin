/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/antiwindows.h
 * windows.h without being shit. Mostly intended for when
 * you want to avoid Win32 as much as possible
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

// i know header guards are missing
// this is so you can still use it after, for example, a single header lib that includes windows.h

#if !defined(_WIN32) && !defined(TR_OS_WINDOWS)
	#error "trippin/antiwindows.h must be included on windows duh"
#endif

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
	#define NOMINMAX
#endif
#ifndef NOGDI
	#define NOGDI
#endif
#ifndef NOUSER
	#define NOUSER
#endif
#ifndef NOSERVICE
	#define NOSERVICE
#endif
#ifndef NOMCX
	#define NOMCX
#endif
#ifndef NOHELP
	#define NOHELP
#endif
#ifndef NOCOMM
	#define NOCOMM
#endif
#ifndef NOLANGUAGE
	#define NOLANGUAGE
#endif
#include <windows.h>

// some bullshit
#ifdef min
	#undef min
#endif
#ifdef max
	#undef max
#endif
#ifdef small
	#undef small
#endif
#ifdef near
	#undef near
#endif
#ifdef far
	#undef far
#endif
#ifdef pascal
	#undef pascal
#endif
#ifdef interface
	#undef interface
#endif

// conflicts with log.h
#ifdef ERROR
	#undef ERROR
#endif

// conflicts with libtrippin errors
// TODO rename all of them for consistency
#ifndef ANTIWINDOWS_KEEP_ERRORS
	#ifdef ERROR_FILE_NOT_FOUND
constexpr long WIN32_ERROR_FILE_NOT_FOUND = ERROR_FILE_NOT_FOUND;
		#undef ERROR_FILE_NOT_FOUND
	#endif
	#ifdef ERROR_ACCESS_DENIED
constexpr long WIN32_ERROR_ACCESS_DENIED = ERROR_ACCESS_DENIED;
		#undef ERROR_ACCESS_DENIED
	#endif
	#ifdef ERROR_FILE_EXISTS
constexpr long WIN32_ERROR_FILE_EXISTS = ERROR_FILE_EXISTS;
		#undef ERROR_FILE_EXISTS
	#endif
	#ifdef ERROR_TOO_MANY_OPEN_FILES
constexpr long WIN32_ERROR_TOO_MANY_OPEN_FILES = ERROR_TOO_MANY_OPEN_FILES;
		#undef ERROR_TOO_MANY_OPEN_FILES
	#endif
	#ifdef ERROR_BROKEN_PIPE
constexpr long WIN32_ERROR_BROKEN_PIPE = ERROR_BROKEN_PIPE;
		#undef ERROR_BROKEN_PIPE
	#endif

// these don't conflict, i just added them here for consistency
#endif

// gdi stuff we'll never use
#ifndef ANTIWINDOWS_KEEP_GDI
	#ifdef Polygon
		#undef Polygon
	#endif
	#ifdef Ellipse
		#undef Ellipse
	#endif
	#ifdef Rectangle
		#undef Rectangle
	#endif
	#ifdef RGB
		#undef RGB
	#endif
#endif
