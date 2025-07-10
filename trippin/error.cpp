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

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define NOSERVICE
	#define NOMCX
	#define NOIME
	#define NOMINMAX
	#include <windows.h>
	#undef ERROR
#else
	#include <errno.h>
#endif

#include "error.hpp"

void tr::FileError::reset_errors()
{
	errno = 0;
	#ifdef _WIN32
	SetLastError(0);
	#endif
}

tr::FileError tr::FileError::from_errno(tr::String patha, tr::String pathb, tr::FileOperation operation)
{
	FileError man = {};
	man.path_a = patha;
	man.path_b = pathb;
	man.op = operation;
	man.errno_code = errno;
	FileErrorType t = FileErrorType::UNKNOWN;

	switch (errno) {
		case ENOENT:        t = FileErrorType::NOT_FOUND;                  break;
		case EACCES:        t = FileErrorType::ACCESS_DENIED;              break;
		case EBUSY:         t = FileErrorType::DEVICE_OR_RESOURCE_BUSY;    break;
		case ENOSPC:        t = FileErrorType::NO_SPACE_LEFT;              break;
		case EEXIST:        t = FileErrorType::FILE_EXISTS;                break;
		case EBADF:         t = FileErrorType::BAD_HANDLE;                 break;
		case EIO:           t = FileErrorType::HARDWARE_ERROR_OR_UNKNOWN;  break;
		case EISDIR:        t = FileErrorType::IS_DIRECTORY;               break;
		case ENOTDIR:       t = FileErrorType::IS_NOT_DIRECTORY;           break;
		case EMFILE:        t = FileErrorType::TOO_MANY_OPEN_FILES;        break;
		case ENFILE:        t = FileErrorType::TOO_MANY_OPEN_FILES;        break;
		case EPIPE:         t = FileErrorType::BROKEN_PIPE;                break;
		case ENAMETOOLONG:  t = FileErrorType::FILENAME_TOO_LONG;          break;
		case EINVAL:        t = FileErrorType::INVALID_ARGUMENT;           break;
		case EROFS:         t = FileErrorType::READ_ONLY_FILESYSTEM;       break;
		case ESPIPE:        t = FileErrorType::ILLEGAL_SEEK;               break;
		case ENOTEMPTY:     t = FileErrorType::DIRECTORY_NOT_EMPTY;        break;
		default:            t = FileErrorType::UNKNOWN;                    break;
	}

	man.type = t;
	return man;
}

#ifdef _WIN32
// Checks Windows' `GetLastError` for errors :)
tr::FileError tr::FileError::from_win32(tr::String path)
{
	// TODO fuck off
}
#endif

tr::String tr::FileError::message()
{
	String operation;
	switch (this->op) {
		case FileOperation::OPEN_FILE:         operation = "couldn't open file";             break;
		case FileOperation::CLOSE_FILE:        operation = "couldn't close file";            break;
		case FileOperation::GET_FILE_POSITION: operation = "couldn't get file position";     break;
		case FileOperation::GET_FILE_LENGTH:   operation = "couldn't get file length";       break;
		case FileOperation::IS_EOF:            operation = "couldn't check if file ended";   break;
		case FileOperation::SEEK_FILE:         operation = "couldn't seek file";             break;
		case FileOperation::REWIND_FILE:       operation = "couldn't rewind file";           break;
		case FileOperation::READ_FILE:         operation = "couldn't read file";             break;
		case FileOperation::FLUSH_FILE:        operation = "couldn't flush file";            break;
		case FileOperation::WRITE_FILE:        operation = "couldn't write file";            break;
		case FileOperation::REMOVE_FILE:       operation = "couldn't remove file";           break;
		case FileOperation::MOVE_FILE:         operation = "couldn't move file";             break;
		case FileOperation::COPY_FILE:         operation = "couldn't copy file";             break;
		case FileOperation::CREATE_DIR:        operation = "couldn't create directory";      break;
		case FileOperation::REMOVE_DIR:        operation = "couldn't remove directory";      break;
		case FileOperation::LIST_DIR:          operation = "couldn't list directory";        break;
		case FileOperation::IS_FILE:           operation = "couldn't check if path is file"; break;
		default:                               operation = "couldn't do file operation";     break;
	}

	String error;
	switch (this->type) {
		case FileErrorType::UNKNOWN:                   error = "unknown error";                         break;
		case FileErrorType::NOT_FOUND:                 error = "no such file or directory";             break;
		case FileErrorType::ACCESS_DENIED:             error = "access denied";                         break;
		case FileErrorType::DEVICE_OR_RESOURCE_BUSY:   error = "device or resource busy";               break;
		case FileErrorType::NO_SPACE_LEFT:             error = "no space left on device";               break;
		case FileErrorType::FILE_EXISTS:               error = "file exists";                           break;
		case FileErrorType::BAD_HANDLE:                error = "bad file handle";                       break;
		case FileErrorType::HARDWARE_ERROR_OR_UNKNOWN: error = "i/o error (might be a hardware issue)"; break;
		case FileErrorType::IS_DIRECTORY:              error = "is directory";                          break;
		case FileErrorType::IS_NOT_DIRECTORY:          error = "is not directory";                      break;
		case FileErrorType::TOO_MANY_OPEN_FILES:       error = "too many open files";                   break;
		case FileErrorType::BROKEN_PIPE:               error = "broken pipe";                           break;
		case FileErrorType::FILENAME_TOO_LONG:         error = "filename too long";                     break;
		case FileErrorType::INVALID_ARGUMENT:          error = "invalid argument";                      break;
		case FileErrorType::READ_ONLY_FILESYSTEM:      error = "read-only filesystem";                  break;
		case FileErrorType::ILLEGAL_SEEK:              error = "illegal seek";                          break;
		case FileErrorType::DIRECTORY_NOT_EMPTY:       error = "directory not empty";                   break;
	}

	// these operations use 2 paths :)
	if (this->op == FileOperation::COPY_FILE || this->op == FileOperation::MOVE_FILE) {
		return tr::sprintf(tr::scratchpad, "%s (source '%s', destination '%s', errno %i): %s",
			operation.buf(), this->path_a.buf(), this->path_b.buf(), this->errno_code, error.buf()
		);
	}
	else {
		return tr::sprintf(tr::scratchpad, "%s (path '%s', errno %i): %s",
			operation.buf(), this->path_a.buf(), this->errno_code, error.buf()
		);
	}
}
