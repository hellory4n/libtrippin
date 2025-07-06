/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/error.hpp
 * I'm not a huge fan of exceptions.
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

#ifndef _TRIPPIN_ERROR_H
#define _TRIPPIN_ERROR_H

#include "common.hpp"
#include "string.hpp"

namespace tr {

// Error interface for making errors lmao.
class Error
{
public:
	virtual ~Error() {}

	// Returns the error message
	virtual String message() = 0;
};

// Basic error interface
class StringError : public Error
{
	String msg;

public:
	StringError(String str) : msg(str) {}

	String message() override { return this->msg; }
};

// Error codes based on POSIX errno.h and WinError.h
enum class FileErrorType : int32
{
	UNKNOWN, NOT_FOUND, ACCESS_DENIED, DEVICE_OR_RESOURCE_BUSY, NO_SPACE_LEFT, FILE_EXISTS, BAD_HANDLE,
	HARDWARE_ERROR, IS_DIRECTORY, IS_NOT_DIRECTORY, TOO_MANY_OPEN_FILES, BROKEN_PIPE, FILENAME_TOO_LONG,
	INVALID_ARGUMENT, READ_ONLY_FILESYSTEM, ILLEGAL_SEEK, DIRECTORY_NOT_EMPTY,
};

// Error for filesystem craps.
class FileError : public Error
{
public:
	String path;
	FileErrorType type;

	// This is required on Linux for some fucking reason
	static void reset_errors();

	// Checks errno.h/WinError.h so it gets the latest error :)
	FileError(String path);

	String message() override;
};

}

#endif
