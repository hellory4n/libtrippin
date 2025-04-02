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
#include <time.h>
#include "libtrippin.h"

void* trippin_new(size_t size)
{
	void* val = calloc(1, size);
	if (val == NULL) {
		printf("libtrippin panic: couldn't allocate memory\n");
		fflush(stdout);
		exit(1);
	}

	// you don't get something and then immediately throw it in the trash
	((TrippinRefHeader*)val)->count = 1;

	// scary!
	return val;
}

void* trippin_reference(void* ptr)
{
	// scary!
	TrippinRefHeader* rc = ((TrippinRefHeader*)(ptr));
	rc->count++;
	return ptr;
}

void trippin_release(void* ptr)
{
	// scary!
	// apparently attribute cleanup returns a pointer to the pointer, not the actual pointer
	TrippinRefHeader* rc = ((TrippinRefHeader*)(*(void**)ptr));
	rc->count--;
	if (rc->count <= 0) {
		free(*((void**)ptr));
	}
}

TrippinContext* trippin_init(const char* log_file)
{
	// no tref because whoever calls this is supposed to deal with it, not us
	TrippinContext* ctx = tnew(TrippinContext);

	strncpy(ctx->log_path, log_file, 256);

	ctx->log_file = fopen(log_file, "w");
	trippin_assert(ctx, ctx->log_file != NULL, "couldn't open %s", log_file);

	trippin_log(ctx, TRIPPIN_LOG_LIB_INFO, "initialized libtrippin %s", TRIPPIN_VERSION);
	return ctx;
}

void trippin_free(TrippinContext* ctx)
{
	fclose(ctx->log_file);

	trippin_log(ctx, TRIPPIN_LOG_LIB_INFO, "deinitialized libtrippin");
}

void trippin_log(TrippinContext* ctx, TrippinLogLevel level, const char* fmt, ...)
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

	fprintf(ctx->log_file, "[%s] %s\n", timestr, buf);
	switch (level) {
	case TRIPPIN_LOG_LIB_INFO:
		printf(TRIPPIN_CONSOLE_COLOR_LIB_INFO "[%s] %s\n" TRIPPIN_CONSOLE_COLOR_RESET, timestr, buf);
		break;
	case TRIPPIN_LOG_INFO:
		printf("[%s] %s\n", timestr, buf);
		break;
	case TRIPPIN_LOG_WARNING:
		printf(TRIPPIN_CONSOLE_COLOR_WARN "[%s] %s\n" TRIPPIN_CONSOLE_COLOR_RESET, timestr, buf);
		break;
	case TRIPPIN_LOG_ERROR:
		printf(TRIPPIN_CONSOLE_COLOR_ERROR "[%s] %s\n" TRIPPIN_CONSOLE_COLOR_RESET, timestr, buf);
		break;
	}
	fflush(ctx->log_file);
	fflush(stdout);

	va_end(args);
}

void trippin_assert(TrippinContext* ctx, bool x, const char* msg, ...)
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

	fprintf(ctx->log_file, "[%s] %s\n", timestr, buf);
	printf(TRIPPIN_CONSOLE_COLOR_ERROR "[%s] failed assert: %s\n" TRIPPIN_CONSOLE_COLOR_RESET, timestr, buf);
	fflush(ctx->log_file);
	fflush(stdout);

	va_end(args);
}

void trippin_panic(TrippinContext* ctx, const char* msg, ...)
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

	fprintf(ctx->log_file, "[%s] %s\n", timestr, buf);
	printf(TRIPPIN_CONSOLE_COLOR_ERROR "[%s] panic: %s\n" TRIPPIN_CONSOLE_COLOR_RESET, timestr, buf);
	fflush(ctx->log_file);
	fflush(stdout);

	va_end(args);
}
