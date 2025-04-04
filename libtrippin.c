/*
 * libtrippin
 *
 * Some utilities craps because the C standard library is tiny
 * More information at https://github.com/hellory4n/libtrippin
 *
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to <https://unlicense.org/>
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// so clang shuts up
#ifdef DEBUG
#include <signal.h>
#endif
#include <time.h>
#include "libtrippin.h"

FILE* trippin_log_file;

void tr_init(const char* log_file)
{
	trippin_log_file = fopen(log_file, "w");
	tr_assert(log_file != NULL, "couldn't open %s", log_file);

	tr_log(TR_LOG_LIB_INFO, "initialized libtr %s", TR_VERSION);
}

void tr_free(void)
{
	fclose(trippin_log_file);

	tr_log(TR_LOG_LIB_INFO, "deinitialized libtr");
}

void tr_log(TrLogLevel level, const char* fmt, ...)
{
	// you understand mechanical hands are the ruler of everything (ah)
	char timestr[32];
	time_t now = time(NULL);
	struct tm* tm_info = localtime(&now);
	strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", tm_info);

	// TODO maybe increase in the future?
	char buf[256];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);

	fprintf(trippin_log_file, "[%s] %s\n", timestr, buf);
	switch (level) {
	case TR_LOG_LIB_INFO:
		printf(TR_CONSOLE_COLOR_LIB_INFO "[%s] %s\n" TR_CONSOLE_COLOR_RESET, timestr, buf);
		break;
	case TR_LOG_INFO:
		printf("[%s] %s\n", timestr, buf);
		break;
	case TR_LOG_WARNING:
		printf(TR_CONSOLE_COLOR_WARN "[%s] %s\n" TR_CONSOLE_COLOR_RESET, timestr, buf);
		break;
	case TR_LOG_ERROR:
		printf(TR_CONSOLE_COLOR_ERROR "[%s] %s\n" TR_CONSOLE_COLOR_RESET, timestr, buf);
		break;
	}
	fflush(trippin_log_file);
	fflush(stdout);

	va_end(args);
}

void tr_assert(bool x, const char* msg, ...)
{
	if (x) {
		return;
	}

	// you understand mechanical hands are the ruler of everything (ah)
	char timestr[32];
	time_t now = time(NULL);
	struct tm* tm_info = localtime(&now);
	strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", tm_info);

	// TODO maybe increase in the future?
	char buf[256];
	va_list args;
	va_start(args, msg);
	vsnprintf(buf, sizeof(buf), msg, args);

	fprintf(trippin_log_file, "[%s] %s\n", timestr, buf);
	printf(TR_CONSOLE_COLOR_ERROR "[%s] failed assert: %s\n" TR_CONSOLE_COLOR_RESET, timestr, buf);
	fflush(trippin_log_file);
	fflush(stdout);

	va_end(args);

	#ifdef DEBUG
	raise(SIGTRAP);
	#else
	exit(1);
	#endif
}

void tr_panic(const char* msg, ...)
{
	// you understand mechanical hands are the ruler of everything (ah)
	char timestr[32];
	time_t now = time(NULL);
	struct tm* tm_info = localtime(&now);
	strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", tm_info);

	// TODO maybe increase in the future?
	char buf[256];
	va_list args;
	va_start(args, msg);
	vsnprintf(buf, sizeof(buf), msg, args);

	fprintf(trippin_log_file, "[%s] %s\n", timestr, buf);
	printf(TR_CONSOLE_COLOR_ERROR "[%s] panic: %s\n" TR_CONSOLE_COLOR_RESET, timestr, buf);
	fflush(trippin_log_file);
	fflush(stdout);

	va_end(args);

	#ifdef DEBUG
	raise(SIGTRAP);
	#else
	exit(1);
	#endif
}

TrArena tr_arena_new(size_t size)
{
	TrArena arena = {.size = size};
	arena.buffer = calloc(1, size);
	return arena;
}

void tr_arena_free(TrArena arena)
{
	free(arena.buffer);
}

void* tr_arena_alloc(TrArena arena, size_t size)
{
	// it's gonna segfault anyway
	// might as well complain instead of mysteriously dying
	size_t end = (size_t)arena.alloc_pos + size;
	if (end >= arena.size) {
		tr_panic("allocation out of bounds of the arena");
	}

	void* data = (void*)((char*)arena.buffer + arena.alloc_pos);
	return data;
}

TrSlice tr_slice_new(TrArena arena, size_t length, size_t elem_size)
{
	TrSlice slicema = {.length = length, .elem_size = elem_size};
	slicema.buffer = tr_arena_alloc(arena, length * elem_size);
	return slicema;
}

void* tr_slice_at(TrSlice slice, size_t idx)
{
	if (idx >= slice.length || idx < 0) {
		printf(TR_CONSOLE_COLOR_ERROR "index out of range: %zu\n" TR_CONSOLE_COLOR_RESET, idx);
		fflush(stdout);

		#ifdef DEBUG
		raise(SIGTRAP);
		#else
		exit(1);
		#endif
	}

	size_t offset = slice.elem_size * idx;
	return (void*)((char*)slice.buffer + offset);
}

TrSlice2D tr_slice2d_new(TrArena arena, size_t width, size_t height, size_t elem_size)
{
	TrSlice2D slicema = {.elem_size = elem_size, .width = width, .height = height};
	slicema.buffer = tr_arena_alloc(arena, width * height * elem_size);
	return slicema;
}

void* tr_slice2d_at(TrSlice2D slice, size_t x, size_t y)
{
	if (x >= slice.width || x < 0 || y >= slice.height || y < 0) {
		printf(TR_CONSOLE_COLOR_ERROR "index out of range: %zu, %zu\n" TR_CONSOLE_COLOR_RESET, x, y);
		fflush(stdout);

		#ifdef DEBUG
		raise(SIGTRAP);
		#else
		exit(1);
		#endif
	}

	size_t offset = slice.elem_size * (slice.width * x + y);
	return (void*)((char*)slice.buffer + offset);
}
