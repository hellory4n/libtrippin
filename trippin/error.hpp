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
	virtual String message() { return "unknown error (tr::Error being used directly)"; };
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
enum class FileErrorType
{
	UNKNOWN,
	NOT_FOUND, ACCESS_DENIED, DEVICE_OR_RESOURCE_BUSY, NO_SPACE_LEFT, FILE_EXISTS, BAD_HANDLE,
	HARDWARE_ERROR_OR_UNKNOWN, IS_DIRECTORY, IS_NOT_DIRECTORY, TOO_MANY_OPEN_FILES, BROKEN_PIPE,
	FILENAME_TOO_LONG, INVALID_ARGUMENT, READ_ONLY_FILESYSTEM, ILLEGAL_SEEK, DIRECTORY_NOT_EMPTY,
};

// This is just for getting the error message lmao.
enum class FileOperation
{
	UNKNOWN,
	OPEN_FILE, CLOSE_FILE, GET_FILE_POSITION, GET_FILE_LENGTH, IS_EOF, SEEK_FILE, REWIND_FILE,
	READ_FILE, FLUSH_FILE, WRITE_FILE, REMOVE_FILE, MOVE_FILE, COPY_FILE, CREATE_DIR, REMOVE_DIR,
	LIST_DIR, IS_FILE,
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
	int win32_code = 0;
	#endif

	FileError() {}
	FileError(String patha, String pathb, FileErrorType errtype, FileOperation operation) :
		path_a(patha), path_b(pathb), type(errtype), op(operation) {}

	// Checks errno for errors :)
	static FileError from_errno(String patha, String pathb, FileOperation operation);

	#ifdef _WIN32
	// Checks Windows' `GetLastError` for errors :)
	static FileError from_win32(String patha, String pathb, FileOperation operation);
	#endif

	// Why.
	static void reset_errors();

	String message() override;
};

// So spicy. E should inherit implement Error
template<typename T, typename E = StringError>
class Result
{
	Either<T, E> value = {};

public:
	Result(const T& val) : value(val) {}
	Result(const E& err) : value(err) {}

	// If true, the result has a value. Else, it has an error.
	bool is_valid() const { return value.is_left(); }

	T& unwrap() const
	{
		if (!this->is_valid()) {
			Error* errormaballs = dynamic_cast<Error*>(&value.right());
			String error;
			if (errormaballs == nullptr) {
				error = "unknown error";
				#ifdef DEBUG
				tr::panic("tr::Result<T, E> is supposed to use tr::Error you distinguished gentleman/lady/everything in between");
				#else
				tr::warn("warning: tr::Result<T, E> is supposed to use tr::Error you distinguished gentleman/lady/everything in between");
				#endif
			}
			else {
				error = errormaballs->message();
			}

			tr::panic("couldn't unwrap tr::Result<T, E>: %s", error.buf());
		}

		return this->value.left();
	}

	E unwrap_err() const
	{
		if (this->is_valid()) {
			tr::panic("couldn't unwrap tr::Result<T, E>'s error, as it's valid");
		}
		return this->value.right();
	}

	// Similar to the `??`/null coalescing operator in modern languages
	const T& value_or(const T& other) const { return this->is_valid() ? this->unwrap() : other; }

	// Calls a function (usually a lambda) depending on whether it's valid or not.
	void match(std::function<void(T& val)> valid_func, std::function<void(E& err)> error_func)
	{
		if (this->is_valid()) valid_func(this->value.left());
		else error_func(this->value.right());
	}
};

// Result for when you don't need the result :D
template<typename E>
class Result<void, E>
{
	Maybe<E> value;

public:
	Result() : value() {}
	Result(const E& err) : value(err) {}

	// If false, it has an error.
	bool is_valid() const { return !this->value.is_valid(); }

	// Pretty much just asserts that it's valid :D
	void unwrap() const
	{
		if (!this->is_valid()) {
			Error* errormaballs = dynamic_cast<Error*>(&value.unwrap());
			String error;
			if (errormaballs == nullptr) {
				error = "unknown error";
				#ifdef DEBUG
				tr::panic("tr::Result<T, E> is supposed to use tr::Error you distinguished gentleman/lady/everything in between");
				#else
				tr::warn("warning: tr::Result<T, E> is supposed to use tr::Error you distinguished gentleman/lady/everything in between");
				#endif
			}
			else {
				error = errormaballs->message();
			}

			tr::panic("couldn't unwrap tr::Result<T, E>: %s", error.buf());
		}
	}

	E unwrap_err() const
	{
		if (this->is_valid()) {
			tr::panic("couldn't unwrap tr::Result<T, E>'s error, as it's valid");
		}
		return this->value.right();
	}

	// Calls a function (usually a lambda) depending on whether it's valid or not.
	void match(std::function<void()> valid_func, std::function<void(E& err)> error_func)
	{
		if (this->is_valid()) valid_func();
		else error_func(this->value.unwrap());
	}
};

}

#endif
