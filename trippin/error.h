/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/error.h
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

#include "trippin/common.h"
#include "trippin/string.h"

namespace tr {

// Error interface for making errors lmao.
class Error
{
public:
	virtual ~Error() {} // shut up

	// Returns the error message
	virtual String message() const = 0;
};

// Basic error interface
class StringError : public Error
{
	String msg;

public:
	StringError()
		: msg("")
	{
	}
	StringError(String str)
		: msg(str)
	{
	}
	// Shorthand for `tr::StringError(tr::fmt(tr::scratchpad(), ...))`
	StringError(const char* fmt, ...);

	String message() const override
	{
		return this->msg;
	}
};

// Error codes based on POSIX errno.h and WinError.h
enum class FileErrorType
{
	UNKNOWN,
	NOT_FOUND,
	ACCESS_DENIED,
	DEVICE_OR_RESOURCE_BUSY,
	NO_SPACE_LEFT,
	FILE_EXISTS,
	BAD_HANDLE,
	HARDWARE_ERROR_OR_UNKNOWN,
	IS_DIRECTORY,
	IS_NOT_DIRECTORY,
	TOO_MANY_OPEN_FILES,
	BROKEN_PIPE,
	FILENAME_TOO_LONG,
	INVALID_ARGUMENT,
	READ_ONLY_FILESYSTEM,
	ILLEGAL_SEEK,
	DIRECTORY_NOT_EMPTY,
};

// This is just for getting the error message lmao.
enum class FileOperation
{
	UNKNOWN,
	OPEN_FILE,
	CLOSE_FILE,
	GET_FILE_POSITION,
	GET_FILE_LENGTH,
	IS_EOF,
	SEEK_FILE,
	REWIND_FILE,
	READ_FILE,
	FLUSH_FILE,
	WRITE_FILE,
	REMOVE_FILE,
	MOVE_FILE,
	CREATE_DIR,
	REMOVE_DIR,
	LIST_DIR,
	IS_FILE,
};

// Error for filesystem craps.
class FileError : public Error
{
public:
	String path_a = "";
	String path_b = "";
	FileErrorType type = FileErrorType::UNKNOWN;
	FileOperation op = FileOperation::UNKNOWN;
	int errno_code = 0;
#ifdef _WIN32
	unsigned win32_code = 0;
#endif

	FileError() {}
	FileError(String patha, String pathb, FileErrorType errtype, FileOperation operation)
		: path_a(patha)
		, path_b(pathb)
		, type(errtype)
		, op(operation)
	{
	}

	// Checks errno for errors :)
	static FileError& from_errno(String patha, String pathb, FileOperation operation);

#ifdef _WIN32
	// Checks Windows' `GetLastError` for errors :)
	static FileError& from_win32(String patha, String pathb, FileOperation operation);
#endif

	// Why.
	static void reset_errors();

	String message() const override;
};

// So spicy.
template<typename T>
class [[nodiscard]] Result
{
	Either<T, const Error*> value = {};

public:
	using Type = T;

	Result(T val)
		: value(val)
	{
	}
	Result(const Error& err)
		: value(&err)
	{
	}

	// If true, the result has a value. Else, it has an error.
	bool is_valid() const
	{
		return value.is_left();
	}

	// If true, the result has an error. Else, it has a value.
	bool is_invalid() const
	{
		return value.is_right();
	}

	T unwrap() const
	{
		if (!this->is_valid()) {
			tr::panic("couldn't unwrap tr::Result<T>: %s", *value.right()->message());
		}

		return this->value.left();
	}

	const Error& unwrap_err() const
	{
		if (this->is_valid()) {
			tr::panic("couldn't unwrap tr::Result<T, E>'s error, as it's valid");
		}
		return *this->value.right();
	}

	// Shorthand for unwrap()
	T operator*() const
	{
		return unwrap();
	}

	// Similar to the `??`/null coalescing operator in modern languages
	const T value_or(const T other) const
	{
		return this->is_valid() ? this->unwrap() : other;
	}
};

// Result for when you don't need the result :D
template<>
class [[nodiscard]] Result<void>
{
	Maybe<const Error*> value;

public:
	using Type = void;

	Result()
		: value()
	{
	}
	Result(const Error& err)
		: value(&err)
	{
	}

	// If true, it has a value. Else, it has an error.
	bool is_valid() const
	{
		return !this->value.is_valid();
	}

	// If true, it has an error. Else, it has a value.
	bool is_invalid() const
	{
		return this->value.is_valid();
	}

	// Pretty much just asserts that it's valid :D
	void unwrap() const
	{
		if (!this->is_valid()) {
			tr::panic("couldn't unwrap tr::Result<T>: %s", *value.unwrap()->message());
		}
	}

	void operator*() const
	{
		unwrap();
	}

	const Error& unwrap_err() const
	{
		if (this->is_valid()) {
			tr::panic("couldn't unwrap tr::Result<T, E>'s error, as it's valid");
		}
		return *this->value.unwrap();
	}
};

}

#endif
