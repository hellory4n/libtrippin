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

tr::FileError tr::FileError::from_errno(tr::String path_a, tr::String path_b, tr::FileOperation op)
{
	FileError man = {};
	man.path_a = path_a;
	man.path_b = path_b;
	man.op = op;
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
	// TODO make this shit
	return "FileError messages not implemented yet lmao";
}
