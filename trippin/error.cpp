/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/error.cpp
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

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define NOSERVICE
	#define NOMCX
	#define NOIME
	#define NOMINMAX
	#include <windows.h>
	#undef ERROR

	#include <cerrno>
#else
	#include <cerrno>
#endif

#include "trippin/error.h"

tr::StringError::StringError(const char* fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);
	this->msg = tr::fmt_args(tr::scratchpad(), fmt, arg);
	va_end(arg);
}

void tr::FileError::reset_errors()
{
	errno = 0;
#ifdef _WIN32
	SetLastError(0);
#endif
}

tr::FileError& tr::FileError::from_errno(tr::String patha, tr::String pathb,
					 tr::FileOperation operation)
{
	FileError& man = tr::scratchpad().make<FileError>();
	man.path_a = patha;
	man.path_b = pathb;
	man.op = operation;
	man.errno_code = errno;
	FileErrorType t = FileErrorType::UNKNOWN;

	switch (errno) {
	case ENOENT:
		t = FileErrorType::NOT_FOUND;
		break;
	case EACCES:
		t = FileErrorType::ACCESS_DENIED;
		break;
	case EBUSY:
		t = FileErrorType::DEVICE_OR_RESOURCE_BUSY;
		break;
	case ENOSPC:
		t = FileErrorType::NO_SPACE_LEFT;
		break;
	case EEXIST:
		t = FileErrorType::FILE_EXISTS;
		break;
	case EBADF:
		t = FileErrorType::BAD_HANDLE;
		break;
	case EIO:
		t = FileErrorType::HARDWARE_ERROR_OR_UNKNOWN;
		break;
	case EISDIR:
		t = FileErrorType::IS_DIRECTORY;
		break;
	case ENOTDIR:
		t = FileErrorType::IS_NOT_DIRECTORY;
		break;
	case EMFILE:
	case ENFILE:
		t = FileErrorType::TOO_MANY_OPEN_FILES;
		break;
	case EPIPE:
		t = FileErrorType::BROKEN_PIPE;
		break;
	case ENAMETOOLONG:
		t = FileErrorType::FILENAME_TOO_LONG;
		break;
	case EINVAL:
		t = FileErrorType::INVALID_ARGUMENT;
		break;
	case EROFS:
		t = FileErrorType::READ_ONLY_FILESYSTEM;
		break;
	case ESPIPE:
		t = FileErrorType::ILLEGAL_SEEK;
		break;
	case ENOTEMPTY:
		t = FileErrorType::DIRECTORY_NOT_EMPTY;
		break;
	}

	man.type = t;
	return man;
}

#ifdef _WIN32
// Checks Windows' `GetLastError` for errors :)
tr::FileError& tr::FileError::from_win32(tr::String patha, tr::String pathb,
					 tr::FileOperation operation)
{
	FileError& man = tr::scratchpad().make<FileError>();
	man.path_a = patha;
	man.path_b = pathb;
	man.op = operation;
	man.win32_code = GetLastError();
	FileErrorType t = FileErrorType::UNKNOWN;

	// i'm not gonna bother aligning this
	// windows doesn't deserve it
	// TODO i think i missed some because win32 is a horrendous affront to mankind
	switch (man.win32_code) {
	case ERROR_FILE_NOT_FOUND:
		t = FileErrorType::NOT_FOUND;
		break;
	case ERROR_ACCESS_DENIED:
		t = FileErrorType::ACCESS_DENIED;
		break;
	case ERROR_SHARING_VIOLATION:
		t = FileErrorType::DEVICE_OR_RESOURCE_BUSY;
		break;
	case ERROR_BUSY:
		t = FileErrorType::DEVICE_OR_RESOURCE_BUSY;
		break;
	case ERROR_DISK_FULL:
		t = FileErrorType::NO_SPACE_LEFT;
		break;
	case ERROR_FILE_EXISTS:
		t = FileErrorType::FILE_EXISTS;
		break;
	case ERROR_ALREADY_EXISTS:
		t = FileErrorType::FILE_EXISTS;
		break;
	case ERROR_INVALID_HANDLE:
		t = FileErrorType::BAD_HANDLE;
		break;
	case ERROR_GEN_FAILURE:
		t = FileErrorType::HARDWARE_ERROR_OR_UNKNOWN;
		break;
	case ERROR_IO_DEVICE:
		t = FileErrorType::HARDWARE_ERROR_OR_UNKNOWN;
		break;
	case ERROR_CANNOT_MAKE:
		t = FileErrorType::IS_DIRECTORY;
		break;
	// TODO its not really that but i dont care
	case ERROR_PATH_NOT_FOUND:
		t = FileErrorType::IS_NOT_DIRECTORY;
		break;
	case ERROR_TOO_MANY_OPEN_FILES:
		t = FileErrorType::TOO_MANY_OPEN_FILES;
		break;
	case ERROR_BROKEN_PIPE:
		t = FileErrorType::BROKEN_PIPE;
		break;
	case ERROR_FILENAME_EXCED_RANGE:
		t = FileErrorType::FILENAME_TOO_LONG;
		break;
	case ERROR_INVALID_PARAMETER:
		t = FileErrorType::INVALID_ARGUMENT;
		break;
	case ERROR_WRITE_PROTECT:
		t = FileErrorType::READ_ONLY_FILESYSTEM;
		break;
	case ERROR_NEGATIVE_SEEK:
		t = FileErrorType::ILLEGAL_SEEK;
		break;
	case ERROR_DIR_NOT_EMPTY:
		t = FileErrorType::DIRECTORY_NOT_EMPTY;
		break;
	}

	man.type = t;
	return man;
}
#endif

tr::String tr::FileError::message() const
{
	String operation;
	switch (this->op) {
	case FileOperation::OPEN_FILE:
		operation = "couldn't open file";
		break;
	case FileOperation::CLOSE_FILE:
		operation = "couldn't close file";
		break;
	case FileOperation::GET_FILE_POSITION:
		operation = "couldn't get file position";
		break;
	case FileOperation::GET_FILE_LENGTH:
		operation = "couldn't get file length";
		break;
	case FileOperation::IS_EOF:
		operation = "couldn't check if file ended";
		break;
	case FileOperation::SEEK_FILE:
		operation = "couldn't seek file";
		break;
	case FileOperation::REWIND_FILE:
		operation = "couldn't rewind file";
		break;
	case FileOperation::READ_FILE:
		operation = "couldn't read file";
		break;
	case FileOperation::FLUSH_FILE:
		operation = "couldn't flush file";
		break;
	case FileOperation::WRITE_FILE:
		operation = "couldn't write file";
		break;
	case FileOperation::REMOVE_FILE:
		operation = "couldn't remove file";
		break;
	case FileOperation::MOVE_FILE:
		operation = "couldn't move file";
		break;
	case FileOperation::CREATE_DIR:
		operation = "couldn't create directory";
		break;
	case FileOperation::REMOVE_DIR:
		operation = "couldn't remove directory";
		break;
	case FileOperation::LIST_DIR:
		operation = "couldn't list directory";
		break;
	case FileOperation::IS_FILE:
		operation = "couldn't check if path is file";
		break;
	default:
		operation = "couldn't do file operation";
		break;
	}

	String error;
	switch (this->type) {
	case FileErrorType::UNKNOWN:
		error = "unknown error";
		break;
	case FileErrorType::NOT_FOUND:
		error = "no such file or directory";
		break;
	case FileErrorType::ACCESS_DENIED:
		error = "access denied";
		break;
	case FileErrorType::DEVICE_OR_RESOURCE_BUSY:
		error = "device or resource busy";
		break;
	case FileErrorType::NO_SPACE_LEFT:
		error = "no space left on device";
		break;
	case FileErrorType::FILE_EXISTS:
		error = "file exists";
		break;
	case FileErrorType::BAD_HANDLE:
		error = "bad file handle";
		break;
	case FileErrorType::HARDWARE_ERROR_OR_UNKNOWN:
		error = "i/o error (might be a hardware issue)";
		break;
	case FileErrorType::IS_DIRECTORY:
		error = "is directory";
		break;
	case FileErrorType::IS_NOT_DIRECTORY:
		error = "is not directory";
		break;
	case FileErrorType::TOO_MANY_OPEN_FILES:
		error = "too many open files";
		break;
	case FileErrorType::BROKEN_PIPE:
		error = "broken pipe";
		break;
	case FileErrorType::FILENAME_TOO_LONG:
		error = "filename too long";
		break;
	case FileErrorType::INVALID_ARGUMENT:
		error = "invalid argument";
		break;
	case FileErrorType::READ_ONLY_FILESYSTEM:
		error = "read-only filesystem";
		break;
	case FileErrorType::ILLEGAL_SEEK:
		error = "illegal seek";
		break;
	case FileErrorType::DIRECTORY_NOT_EMPTY:
		error = "directory not empty";
		break;
	}

// these operations use 2 paths :)
#ifndef _WIN32
	if (this->op == FileOperation::MOVE_FILE) {
		return tr::fmt(tr::scratchpad(), "%s (source '%s', destination '%s', errno %i): %s",
			       operation.buf(), this->path_a.buf(), this->path_b.buf(),
			       this->errno_code, error.buf());
	}
	return tr::fmt(tr::scratchpad(), "%s (path '%s', errno %i): %s", operation.buf(),
		       this->path_a.buf(), this->errno_code, error.buf());

#else
	// on windows we currently use both errno and win32
	if (this->errno_code != 0) {
		if (this->op == FileOperation::MOVE_FILE) {
			return tr::fmt(tr::scratchpad(),
				       "%s (source '%s', destination '%s', errno %i): %s",
				       operation.buf(), this->path_a.buf(), this->path_b.buf(),
				       this->errno_code, error.buf());
		}
		else {
			return tr::fmt(tr::scratchpad(), "%s (path '%s', errno %i): %s",
				       operation.buf(), this->path_a.buf(), this->errno_code,
				       error.buf());
		}
	}
	else {
		if (this->op == FileOperation::MOVE_FILE) {
			return tr::fmt(tr::scratchpad(),
				       "%s (source '%s', destination '%s', win32 error %i): %s",
				       operation.buf(), this->path_a.buf(), this->path_b.buf(),
				       this->win32_code, error.buf());
		}
		else {
			return tr::fmt(tr::scratchpad(), "%s (path '%s', win32 error %i): %s",
				       operation.buf(), this->path_a.buf(), this->win32_code,
				       error.buf());
		}
	}
#endif
}
