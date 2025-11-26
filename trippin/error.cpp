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

#include "trippin/error.h"

#include <cerrno>

#ifdef TR_OS_WINDOWS
	#include "trippin/antiwindows.h"
#endif
#include "trippin/memory.h"
#include "trippin/string.h"
#include "trippin/util.h"
/* clang-format off */
// man
#undef _TRIPPIN_BITS_STATE_H
#define _TR_BULLSHIT_SO_THAT_IT_WORKS
#include "trippin/bits/state.h"
/* clang-format on */

tr::TempString tr::Error::message() const
{
	return tr::error_message(type, args);
}

bool tr::register_error_type(ErrorType id, TempString (*msg_func)(ErrorArgs args), bool override)
{
	// quite the mouthful
	Maybe<TempString (*&)(ErrorArgs)> perchance = _tr::error_table().try_get(id);
	if (perchance.is_valid()) {
		if (override) {
			perchance.unwrap() = msg_func;
		}
		return false;
	}

	_tr::error_table()[id] = msg_func;
	return true;
}

tr::TempString tr::error_message(tr::ErrorType id, tr::ErrorArgs args)
{
	// quite the mouthful
	Maybe<TempString (*&)(ErrorArgs)> perchance = _tr::error_table().try_get(id);
	if (perchance.is_invalid()) {
		tr::panic("error type %lu doesn't exist", static_cast<uint64>(id));
	}
	return perchance.unwrap()(args);
}

void tr::_reset_os_errors()
{
	errno = 0;
#ifdef _WIN32
	SetLastError(0);
#endif
}

tr::ErrorType tr::_trippin_error_from_errno()
{
	ErrorType t;
	switch (errno) {
	case ENOENT:
		t = ERROR_FILE_NOT_FOUND;
		break;
	case EACCES:
		t = ERROR_ACCESS_DENIED;
		break;
	case EBUSY:
		t = ERROR_DEVICE_OR_RESOURCE_BUSY;
		break;
	case ENOSPC:
		t = ERROR_NO_SPACE_LEFT;
		break;
	case EEXIST:
		t = ERROR_FILE_EXISTS;
		break;
	case EBADF:
		t = ERROR_BAD_HANDLE;
		break;
	case EISDIR:
		t = ERROR_IS_DIRECTORY;
		break;
	case ENOTDIR:
		t = ERROR_IS_NOT_DIRECTORY;
		break;
	case EMFILE:
	case ENFILE:
		t = ERROR_TOO_MANY_OPEN_FILES;
		break;
	case EPIPE:
		t = ERROR_BROKEN_PIPE;
		break;
	case ENAMETOOLONG:
		t = ERROR_FILENAME_TOO_LONG;
		break;
	case EINVAL:
		t = ERROR_INVALID_ARGUMENT;
		break;
	case EROFS:
		t = ERROR_READ_ONLY_FILESYSTEM;
		break;
	case ESPIPE:
		t = ERROR_ILLEGAL_SEEK;
		break;
	case ENOTEMPTY:
		t = ERROR_DIRECTORY_NOT_EMPTY;
		break;
	case EIO:
	default: // TODO the rest of errno
		t = ERROR_HARDWARE_ERROR_OR_UNKNOWN;
		break;
	}
	return t;
}

#ifdef TR_OS_WINDOWS
tr::ErrorType tr::_trippin_error_from_win32()
{
	ErrorType t;

	// TODO i think i missed some because win32 is a horrendous affront to mankind
	// errors prefixed with WIN32 are like that so that they don't conflict with libtrippin's
	// own errors
	switch (GetLastError()) {
	case WIN32_ERROR_FILE_NOT_FOUND:
		t = ERROR_FILE_NOT_FOUND;
		break;
	case WIN32_ERROR_ACCESS_DENIED:
		t = ERROR_ACCESS_DENIED;
		break;
	case ERROR_SHARING_VIOLATION:
	case ERROR_BUSY:
		t = ERROR_DEVICE_OR_RESOURCE_BUSY;
		break;
	case ERROR_DISK_FULL:
		t = ERROR_NO_SPACE_LEFT;
		break;
	case WIN32_ERROR_FILE_EXISTS:
	case ERROR_ALREADY_EXISTS:
		t = ERROR_FILE_EXISTS;
		break;
	case ERROR_INVALID_HANDLE:
		t = ERROR_BAD_HANDLE;
		break;
	case ERROR_GEN_FAILURE:
	case ERROR_IO_DEVICE:
		t = ERROR_HARDWARE_ERROR_OR_UNKNOWN;
		break;
	case ERROR_CANNOT_MAKE:
		t = ERROR_IS_DIRECTORY;
		break;
	// TODO its not really that but i dont care
	case ERROR_PATH_NOT_FOUND:
		t = ERROR_IS_NOT_DIRECTORY;
		break;
	case WIN32_ERROR_TOO_MANY_OPEN_FILES:
		t = ERROR_TOO_MANY_OPEN_FILES;
		break;
	case WIN32_ERROR_BROKEN_PIPE:
		t = ERROR_BROKEN_PIPE;
		break;
	case ERROR_FILENAME_EXCED_RANGE:
		t = ERROR_FILENAME_TOO_LONG;
		break;
	case ERROR_INVALID_PARAMETER:
		t = ERROR_INVALID_ARGUMENT;
		break;
	case ERROR_WRITE_PROTECT:
		t = ERROR_READ_ONLY_FILESYSTEM;
		break;
	case ERROR_NEGATIVE_SEEK:
		t = ERROR_ILLEGAL_SEEK;
		break;
	case ERROR_DIR_NOT_EMPTY:
		t = ERROR_DIRECTORY_NOT_EMPTY;
		break;
	}
	return t;
}
#endif

static inline tr::TempString
_real_file_errmsg(tr::FileOperation op, tr::String error, tr::String path_a, tr::String path_b = {})
{
	tr::String operation;
	switch (op) {
	case tr::FileOperation::OPEN_FILE:
		operation = "couldn't open file";
		break;
	case tr::FileOperation::CLOSE_FILE:
		operation = "couldn't close file";
		break;
	case tr::FileOperation::GET_FILE_POSITION:
		operation = "couldn't get file position";
		break;
	case tr::FileOperation::GET_FILE_LENGTH:
		operation = "couldn't get file length";
		break;
	case tr::FileOperation::IS_EOF:
		operation = "couldn't check if file ended";
		break;
	case tr::FileOperation::SEEK_FILE:
		operation = "couldn't seek file";
		break;
	case tr::FileOperation::REWIND_FILE:
		operation = "couldn't rewind file";
		break;
	case tr::FileOperation::READ_FILE:
		operation = "couldn't read file";
		break;
	case tr::FileOperation::FLUSH_FILE:
		operation = "couldn't flush file";
		break;
	case tr::FileOperation::WRITE_FILE:
		operation = "couldn't write file";
		break;
	case tr::FileOperation::REMOVE_FILE:
		operation = "couldn't remove file";
		break;
	case tr::FileOperation::MOVE_FILE:
		operation = "couldn't move file";
		break;
	case tr::FileOperation::CREATE_DIR:
		operation = "couldn't create directory";
		break;
	case tr::FileOperation::REMOVE_DIR:
		operation = "couldn't remove directory";
		break;
	case tr::FileOperation::LIST_DIR:
		operation = "couldn't list directory";
		break;
	case tr::FileOperation::IS_FILE:
		operation = "couldn't check if path is file";
		break;
	default:
		operation = "couldn't do file operation";
		break;
	}

	// not all file operations use 2 paths
	if (path_b == "") {
		return tr::tmp_fmt("%s from '%s': %s", *operation, *path_a, *error);
	}
	else {
		return tr::tmp_fmt(
			"%s from '%s' to '%s': %s", *operation, *path_a, *path_b, *error
		);
	}
}

tr::TempString tr::errmsg_file_not_found(tr::ErrorArgs args)
{
	return _real_file_errmsg(
		static_cast<FileOperation>(args[0].i32), "no such file or directory", args[16].str
	);
}

tr::TempString tr::errmsg_access_denied(tr::ErrorArgs args)
{
	return _real_file_errmsg(
		static_cast<FileOperation>(args[0].i32), "access denied", args[1].str
	);
}

tr::TempString tr::errmsg_device_or_resource_busy(tr::ErrorArgs args)
{
	return _real_file_errmsg(
		static_cast<FileOperation>(args[0].i32), "device or resource busy", args[1].str
	);
}

tr::TempString tr::errmsg_no_space_left(tr::ErrorArgs args)
{
	return _real_file_errmsg(
		static_cast<FileOperation>(args[0].i32), "no space left on device", args[1].str
	);
}

tr::TempString tr::errmsg_file_exists(tr::ErrorArgs args)
{
	return _real_file_errmsg(
		static_cast<FileOperation>(args[0].i32), "file exists", args[1].str
	);
}

tr::TempString tr::errmsg_bad_handle(tr::ErrorArgs args)
{
	return _real_file_errmsg(
		static_cast<FileOperation>(args[0].i32), "bad file handle", args[1].str
	);
}

tr::TempString tr::errmsg_hardware_error_or_unknown(tr::ErrorArgs args)
{
	return _real_file_errmsg(
		static_cast<FileOperation>(args[0].i32), "I/O error (hardware issue?)", args[1].str
	);
}

tr::TempString tr::errmsg_is_directory(tr::ErrorArgs args)
{
	return _real_file_errmsg(
		static_cast<FileOperation>(args[0].i32), "is directory", args[1].str
	);
}

tr::TempString tr::errmsg_is_not_directory(tr::ErrorArgs args)
{
	return _real_file_errmsg(
		static_cast<FileOperation>(args[0].i32), "is not directory", args[1].str
	);
}

tr::TempString tr::errmsg_too_many_open_files(tr::ErrorArgs args)
{
	return _real_file_errmsg(
		static_cast<FileOperation>(args[0].i32), "too many open files", args[1].str
	);
}

tr::TempString tr::errmsg_broken_pipe(tr::ErrorArgs args)
{
	return _real_file_errmsg(
		static_cast<FileOperation>(args[0].i32), "broken pipe", args[1].str
	);
}

tr::TempString tr::errmsg_filename_too_long(tr::ErrorArgs args)
{
	return _real_file_errmsg(
		static_cast<FileOperation>(args[0].i32), "filename too long", args[1].str
	);
}

tr::TempString tr::errmsg_invalid_argument(tr::ErrorArgs args)
{
	return _real_file_errmsg(
		static_cast<FileOperation>(args[0].i32), "invalid argument", args[1].str
	);
}

tr::TempString tr::errmsg_read_only_filesystem(tr::ErrorArgs args)
{
	return _real_file_errmsg(
		static_cast<FileOperation>(args[0].i32), "read-only filesystem", args[1].str
	);
}

tr::TempString tr::errmsg_illegal_seek(tr::ErrorArgs args)
{
	return _real_file_errmsg(
		static_cast<FileOperation>(args[0].i32), "illegal seek", args[1].str
	);
}

tr::TempString tr::errmsg_directory_not_empty(tr::ErrorArgs args)
{
	return _real_file_errmsg(
		static_cast<FileOperation>(args[0].i32), "directory not empty", args[1].str
	);
}
