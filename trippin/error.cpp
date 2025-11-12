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

#include "trippin/memory.h"
#include "trippin/string.h"
#include "trippin/util.h"
#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define NOSERVICE
	#define NOMCX
	#define NOIME
	#define NOMINMAX
	#include <windows.h>
	#undef ERROR
	#undef TRANSPARENT
#endif

#include <cerrno>

#include "trippin/error.h"

// TODO there's no way this is thread safe

namespace tr {

extern Arena core_arena;
static HashMap<ErrorType, String (*)(ErrorArgs args)> _error_table;

}

// you can't control the order in which static vars are initialized, so instead of initializing
// _error_table where it's defined, we initialize it when the first error is registered
static inline void _error_table_init()
{
	static bool initialized = false;
	if (!initialized) {
		tr::_error_table =
			tr::HashMap<tr::ErrorType, tr::String (*)(tr::ErrorArgs)>{tr::core_arena};
	}
}

tr::String tr::Error::message() const
{
	return tr::error_message(type, args);
}

bool tr::register_error_type(ErrorType id, String (*msg_func)(ErrorArgs args), bool override)
{
	_error_table_init();

	// quite the mouthful
	Maybe<String (*&)(ErrorArgs)> perchance = _error_table.try_get(id);
	if (perchance.is_valid()) {
		if (override) {
			perchance.unwrap() = msg_func;
		}
		return false;
	}

	_error_table[id] = msg_func;
	return true;
}

tr::String tr::error_message(tr::ErrorType id, tr::ErrorArgs args)
{
	// quite the mouthful
	Maybe<String (*&)(ErrorArgs)> perchance = _error_table.try_get(id);
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

#ifdef _WIN32
// Checks Windows' `GetLastError` for errors :)
tr::FileError
tr::FileError::from_win32(tr::String patha, tr::String pathb, tr::FileOperation operation)
{
	FileError man{};
	man.path_a = patha;
	man.path_b = pathb;
	man.op = operation;
	man.win32_code = GetLastError();
	FileErrorType t = FileErrorType::UNKNOWN;

	// TODO i think i missed some because win32 is a horrendous affront to mankind
	switch (man.win32_code) {
	case ERROR_FILE_NOT_FOUND:
		t = FileErrorType::NOT_FOUND;
		break;
	case ERROR_ACCESS_DENIED:
		t = FileErrorType::ACCESS_DENIED;
		break;
	case ERROR_SHARING_VIOLATION:
	case ERROR_BUSY:
		t = FileErrorType::DEVICE_OR_RESOURCE_BUSY;
		break;
	case ERROR_DISK_FULL:
		t = FileErrorType::NO_SPACE_LEFT;
		break;
	case ERROR_FILE_EXISTS:
	case ERROR_ALREADY_EXISTS:
		t = FileErrorType::FILE_EXISTS;
		break;
	case ERROR_INVALID_HANDLE:
		t = FileErrorType::BAD_HANDLE;
		break;
	case ERROR_GEN_FAILURE:
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

static inline tr::String
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
		return tr::fmt(tr::scratchpad(), "%s from '%s': %s", *operation, *path_a, *error);
	}
	else {
		return tr::fmt(
			tr::scratchpad(), "%s from '%s' to '%s': %s", *operation, *path_a, *path_b,
			*error
		);
	}
}

tr::String tr::errmsg_file_not_found(tr::ErrorArgs args)
{
	return _real_file_errmsg(
		static_cast<FileOperation>(args[0].i32), "no such file or directory", args[16].str
	);
}

tr::String tr::errmsg_access_denied(tr::ErrorArgs args)
{
	return _real_file_errmsg(
		static_cast<FileOperation>(args[0].i32), "access denied", args[1].str
	);
}

tr::String tr::errmsg_device_or_resource_busy(tr::ErrorArgs args)
{
	return _real_file_errmsg(
		static_cast<FileOperation>(args[0].i32), "device or resource busy", args[1].str
	);
}

tr::String tr::errmsg_no_space_left(tr::ErrorArgs args)
{
	return _real_file_errmsg(
		static_cast<FileOperation>(args[0].i32), "no space left on device", args[1].str
	);
}

tr::String tr::errmsg_file_exists(tr::ErrorArgs args)
{
	return _real_file_errmsg(
		static_cast<FileOperation>(args[0].i32), "file exists", args[1].str
	);
}

tr::String tr::errmsg_bad_handle(tr::ErrorArgs args)
{
	return _real_file_errmsg(
		static_cast<FileOperation>(args[0].i32), "bad file handle", args[1].str
	);
}

tr::String tr::errmsg_hardware_error_or_unknown(tr::ErrorArgs args)
{
	return _real_file_errmsg(
		static_cast<FileOperation>(args[0].i32), "I/O error (hardware issue?)", args[1].str
	);
}

tr::String tr::errmsg_is_directory(tr::ErrorArgs args)
{
	return _real_file_errmsg(
		static_cast<FileOperation>(args[0].i32), "is directory", args[1].str
	);
}

tr::String tr::errmsg_is_not_directory(tr::ErrorArgs args)
{
	return _real_file_errmsg(
		static_cast<FileOperation>(args[0].i32), "is not directory", args[1].str
	);
}

tr::String tr::errmsg_too_many_open_files(tr::ErrorArgs args)
{
	return _real_file_errmsg(
		static_cast<FileOperation>(args[0].i32), "too many open files", args[1].str
	);
}

tr::String tr::errmsg_broken_pipe(tr::ErrorArgs args)
{
	return _real_file_errmsg(
		static_cast<FileOperation>(args[0].i32), "broken pipe", args[1].str
	);
}

tr::String tr::errmsg_filename_too_long(tr::ErrorArgs args)
{
	return _real_file_errmsg(
		static_cast<FileOperation>(args[0].i32), "filename too long", args[1].str
	);
}

tr::String tr::errmsg_invalid_argument(tr::ErrorArgs args)
{
	return _real_file_errmsg(
		static_cast<FileOperation>(args[0].i32), "invalid argument", args[1].str
	);
}

tr::String tr::errmsg_read_only_filesystem(tr::ErrorArgs args)
{
	return _real_file_errmsg(
		static_cast<FileOperation>(args[0].i32), "read-only filesystem", args[1].str
	);
}

tr::String tr::errmsg_illegal_seek(tr::ErrorArgs args)
{
	return _real_file_errmsg(
		static_cast<FileOperation>(args[0].i32), "illegal seek", args[1].str
	);
}

tr::String tr::errmsg_directory_not_empty(tr::ErrorArgs args)
{
	return _real_file_errmsg(
		static_cast<FileOperation>(args[0].i32), "directory not empty", args[1].str
	);
}
