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

#include <type_traits>

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
#ifdef TR_OS_WINDOWS
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
	static FileError from_errno(String patha, String pathb, FileOperation operation);

#ifdef TR_OS_WINDOWS
	// Checks Win32's `GetLastError` for errors :)
	static FileError from_win32(String patha, String pathb, FileOperation operation);
#endif

	// Why.
	static void reset_errors();

	String message() const override;
};

// So spicy.
template<typename T>
requires(!std::is_reference_v<T>) // TODO fucking fix it
class [[nodiscard]] Result
{
	// store errors inline through type erasure type shit
	// inheritance requires ptrs, heap memory is overkill for errors (+ don't think there's any
	// clean way to manage an error arena or whatever)
	alignas(std::max_align_t) List<MAX_ERROR_SIZE, byte> _inline_error{};
	T _value{};
	bool _valid = false;

public:
	using Type = T;

	Result(T val)
		: _value(val)
		, _valid(true)
	{
	}
	template<typename E>
	requires(std::is_base_of_v<Error, E> && sizeof(E) <= MAX_ERROR_SIZE)
	Result(const E& err)
		: _valid(true)
	{
		// if only someone defined that behavior...
		*reinterpret_cast<E*>(*_inline_error) = err;
	}

	// If true, the result has a value. Else, it has an error.
	bool is_valid() const
	{
		return _valid;
	}

	// If true, the result has an error. Else, it has a value.
	bool is_invalid() const
	{
		return !_valid;
	}

	T unwrap() const
	{
		if (is_invalid()) {
			// yea
			const Error* err = reinterpret_cast<const Error*>(*_inline_error);
			tr::panic("couldn't unwrap tr::Result<T>: %s", *err->message());
		}

		return _value;
	}

	const Error& unwrap_err() const
	{
		if (is_valid()) {
			tr::panic("couldn't unwrap tr::Result<T>'s error, as it's valid");
		}
		return *reinterpret_cast<const Error*>(*_inline_error);
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
	// store errors inline through type erasure type shit
	// inheritance requires ptrs, heap memory is overkill for errors (+ don't think there's any
	// clean way to manage an error arena or whatever)
	alignas(std::max_align_t) List<MAX_ERROR_SIZE, byte> _inline_error{};
	bool _valid = false;

public:
	using Type = void;

	Result()
		: _valid(false)
	{
	}
	template<typename E>
	requires(std::is_base_of_v<Error, E> && sizeof(E) <= MAX_ERROR_SIZE)
	Result(const E& err)
		: _valid(true)
	{
		// if only someone defined that behavior...
		*reinterpret_cast<E*>(*_inline_error) = err;
	}

	// If true, it has a value. Else, it has an error.
	bool is_valid() const
	{
		return _valid;
	}

	// If true, it has an error. Else, it has a value.
	bool is_invalid() const
	{
		return !_valid;
	}

	// Pretty much just asserts that it's valid :D
	void unwrap() const
	{
		if (is_invalid()) {
			// yea
			const Error* err = reinterpret_cast<const Error*>(*_inline_error);
			tr::panic("couldn't unwrap tr::Result<void>: %s", *err->message());
		}
	}

	void operator*() const
	{
		unwrap();
	}

	const Error& unwrap_err() const
	{
		if (is_valid()) {
			tr::panic("couldn't unwrap tr::Result<void>'s error, as it's valid");
		}
		return *reinterpret_cast<const Error*>(*_inline_error);
	}
};

}

#endif
