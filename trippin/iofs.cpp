/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/iofs.cpp
 * Stream and filesystem APIs
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

#include "trippin/common.h"
#include "trippin/error.h"

// :(
// TODO macOS exists
// though macOS should be easier as it supports posix
#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define NOSERVICE
	#define NOMCX
	#define NOIME
	// mingw gcc already defines that by default??
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
	#include <windows.h>

	// conflicts :D
	#undef ERROR

	#include <cstdio>

	// windows and its consequences have been a disaster for the human race
	#include <direct.h>
#else
	#include <cerrno>
	#include <cstdio>
	#include <cstdlib>

	#include <dirent.h>
	#include <sys/stat.h>
	#include <sys/types.h>
	#include <unistd.h>
#endif

#include "trippin/iofs.h"
#include "trippin/log.h"

namespace tr {

extern Arena core_arena;

String exe_dir;
String appdata_dir;

String app_dir;
String user_dir;

}

// TODO this was written before TR_TRY and all that crap
// so like, use that?

tr::Result<tr::String> tr::Reader::read_string(tr::Arena& arena, int64 length)
{
	String str(arena, static_cast<usize>(length));
	TR_TRY_ASSIGN(int64 read, this->read_bytes(str.buf(), sizeof(char), length));

	if (read != int64(length)) {
		return tr::scratchpad().make_ref<StringError>(
			"expected %zu bytes, got %li (might be EOF)", length, read
		);
	}
	return str;
}

tr::Result<tr::String> tr::Reader::read_line(Arena& arena)
{
	Array<char> linema(tr::scratchpad(), 0);

	while (true) {
		char byte = '\0';
		TR_TRY_ASSIGN(int64 read, this->read_bytes(&byte, sizeof(char), 1));

		// eof? idfk man
		if (read == 0) {
			break;
		}

		// i love unixcxfukc
		if (byte == '\n') {
			break;
		}

		// windows :(
		if (byte == '\r') {
			char next_byte = '\0';
			TR_TRY_ASSIGN(read, this->read_bytes(&next_byte, sizeof(char), 1));

			// eof still counts
			if (read == 0 || next_byte == '\n') {
				break;
			}
		}

		// normal character
		linema.add(byte);
	}

	if (linema.len() == 0) {
		return String("");
	}
	return String(arena, linema.buf(), linema.len() + 1);
}

tr::Result<tr::Array<uint8>> tr::Reader::read_all_bytes(tr::Arena& arena)
{
	TR_TRY_ASSIGN(int64 length, this->len());

	Array<uint8> man(arena, static_cast<usize>(length));
	TR_TRY(this->read_bytes(man.buf(), sizeof(uint8), length));
	return man;
}

tr::Result<tr::String> tr::Reader::read_all_text(tr::Arena& arena)
{
	TR_TRY_ASSIGN(int64 length, this->len());

	String man(arena, static_cast<usize>(length));
	TR_TRY(this->read_bytes(man.buf(), sizeof(char), length));
	return man;
}

tr::Result<void> tr::Writer::write_string(tr::String str)
{
	Array<uint8> manfuckyou(reinterpret_cast<uint8*>(str.buf()), str.len());
	return this->write_bytes(manfuckyou);
}

tr::Result<void> tr::Writer::printf(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	String str = tr::fmt_args(tr::scratchpad(), fmt, args);
	va_end(args);

	return this->write_string(str);
}

tr::Result<void> tr::Writer::println(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	tr::String str = tr::fmt_args(tr::scratchpad(), fmt, args);
	va_end(args);

	TR_TRY(this->write_string(str));
	return this->write_string("\n");
}

tr::String tr::path(tr::Arena& arena, tr::String path)
{
	if (path.starts_with("app://")) {
		String pathfrfr = path.substr(tr::scratchpad(), sizeof("app://") - 1, path.len());
		return tr::fmt(
			arena, "%s/%s/%s", tr::exe_dir.buf(), tr::app_dir.buf(), pathfrfr.buf()
		);
	}

	if (path.starts_with("user://")) {
		String pathfrfr = path.substr(tr::scratchpad(), sizeof("user://") - 1, path.len());
		return tr::fmt(
			arena, "%s/%s/%s", tr::appdata_dir.buf(), tr::user_dir.buf(), pathfrfr.buf()
		);
	}

	return path.duplicate(arena);
}

void tr::set_paths(tr::String appdir, tr::String userdir)
{
	tr::app_dir = appdir.duplicate(tr::core_arena);
	tr::user_dir = userdir.duplicate(tr::core_arena);
}

// TODO use only windows APIs (massive pain in the ass)
// TODO maybe at some point just make this be a fancy wrapper for some other library
// TODO seek help

#ifdef _WIN32
/*
 * WINDOWS IMPLEMENTATION
 */

// you know how hard it is to remember LPCWSTR???? and visual studio takes a week to show the
// autocomplete
using WinStrConst = LPCWSTR;
using WinStrMut = LPWSTR;

// windows uses utf-16 :(
static WinStrConst from_trippin_to_win32_str(tr::String str)
{
	// conveniently microsoft knows this is torture and gives a function for this exact purpose
	int size = MultiByteToWideChar(CP_UTF8, 0, str.buf(), -1, nullptr, 0);
	// TODO idk if it can go negative but the headers don't have documentation, and the search
	// online button uses bing
	// let's just say, that this table right here, is bill gates!
	// *smashes table* YEAHHHHHHHHHHHHHH https://www.youtube.com/watch?v=WGFLPbpdMS8
	TR_ASSERT_MSG(size != 0, "blame it on windows");

	WinStrMut new_str = static_cast<WinStrMut>(
		tr::scratchpad().alloc(static_cast<usize>(size + 1) * sizeof(wchar_t))
	);
	int result = MultiByteToWideChar(CP_UTF8, 0, str.buf(), -1, new_str, size);
	TR_ASSERT_MSG(result != 0, "blame it on windows");
	return new_str;
}

static tr::String from_win32_to_trippin_str(WinStrConst str)
{
	int size = WideCharToMultiByte(CP_UTF8, 0, str, -1, nullptr, 0, nullptr, nullptr);
	TR_ASSERT_MSG(size != 0, "blame it on windows");

	tr::String new_str(tr::scratchpad(), static_cast<usize>(size));
	int result =
		WideCharToMultiByte(CP_UTF8, 0, str, -1, new_str.buf(), size, nullptr, nullptr);
	TR_ASSERT_MSG(result != 0, "blame it on windows");
	return new_str;
}

tr::Result<tr::File&> tr::File::open(tr::Arena& arena, tr::String path, FileMode mode)
{
	FileError::reset_errors();

	// get mode
	WinStrConst modefrfr = L"";
	switch (mode) {
	// on text mode windows does evil fuckery that we don't want
	// we want everything to be unix like
	// so we have to use binary mode
	case FileMode::READ_TEXT:
	case FileMode::READ_BINARY:
		modefrfr = L"rb";
		break;

	case FileMode::WRITE_TEXT:
	case FileMode::WRITE_BINARY:
		modefrfr = L"wb";
		break;

	case FileMode::READ_WRITE_TEXT:
	case FileMode::READ_WRITE_BINARY:
		modefrfr = L"rb+";
		break;

	default:
		modefrfr = L"";
		break;
	}

	File& file = arena.make_ref<File>();
	// normal fopen gives an error on visual studio??
	errno_t ohno = _wfopen_s(
		reinterpret_cast<FILE**>(&file.fptr), from_trippin_to_win32_str(path), modefrfr
	);
	if (ohno != 0) {
		return FileError::from_errno(path, "", FileOperation::OPEN_FILE);
	}

	file.is_std = false;
	file.mode = mode;
	file.path = path.duplicate(arena);

	// get length :)))))))))
	_fseeki64(static_cast<FILE*>(file.fptr), 0, SEEK_END);
	file.length = _ftelli64(static_cast<FILE*>(file.fptr));
	::rewind(static_cast<FILE*>(file.fptr));

	return file;
}

void tr::File::close()
{
	FileError::reset_errors();

	// is_std exists so it doesn't close tr::std_out and company
	if (!this->is_std && this->fptr != nullptr) {
		fclose(static_cast<FILE*>(this->fptr));
	}
	this->fptr = nullptr;
}

tr::Result<int64> tr::File::position()
{
	FileError::reset_errors();

	int64 pos = _ftelli64(static_cast<FILE*>(this->fptr));
	if (pos < 0) {
		return FileError::from_errno(this->path, "", FileOperation::GET_FILE_POSITION);
	}
	return pos;
}

tr::Result<int64> tr::File::len()
{
	FileError::reset_errors();
	return this->length;
}

tr::Result<bool> tr::File::eof()
{
	FileError::reset_errors();

	return feof(static_cast<FILE*>(this->fptr)) != 0;
}

tr::Result<void> tr::File::seek(int64 bytes, tr::SeekFrom from)
{
	FileError::reset_errors();

	int whence = SEEK_CUR;
	switch (from) {
	case SeekFrom::START:
		whence = SEEK_SET;
		break;
	case SeekFrom::CURRENT:
		whence = SEEK_CUR;
		break;
	case SeekFrom::END:
		whence = SEEK_END;
		break;
	}

	int i = _fseeki64(static_cast<FILE*>(this->fptr), bytes, whence);
	if (i != 0) {
		return FileError::from_errno(this->path, "", FileOperation::SEEK_FILE);
	}
	return {};
}

tr::Result<void> tr::File::rewind()
{
	FileError::reset_errors();

	::rewind(static_cast<FILE*>(this->fptr));
	if (errno != 0) {
		return FileError::from_errno(this->path, "", FileOperation::REWIND_FILE);
	}
	return {};
}

tr::Result<int64> tr::File::read_bytes(void* out, int64 size, int64 items)
{
	FileError::reset_errors();
	TR_TRY_ASSERT(
		out != nullptr, tr::scratchpad().make_ref<StringError>(
					"you dumbass it's supposed to go somewhere if you don't "
					"want to use it use File::seek() dumbass"
				)
	);
	TR_TRY_ASSERT(
		this->can_read(),
		tr::scratchpad().make_ref<FileError>(
			this->path, "", FileErrorType::ACCESS_DENIED, FileOperation::READ_FILE
		)
	);

	usize bytes =
		fread(out, static_cast<usize>(size), static_cast<usize>(items),
		      static_cast<FILE*>(this->fptr));
	if (errno != 0) {
		return FileError::from_errno(this->path, "", FileOperation::READ_FILE);
	}
	return static_cast<int64>(bytes);
}

tr::Result<void> tr::File::flush()
{
	FileError::reset_errors();

	int i = fflush(static_cast<FILE*>(this->fptr));
	if (i == EOF) {
		return FileError::from_errno(this->path, "", FileOperation::FLUSH_FILE);
	}
	return {};
}

tr::Result<void> tr::File::write_bytes(Array<uint8> bytes)
{
	FileError::reset_errors();
	TR_TRY_ASSERT(
		this->can_write(),
		tr::scratchpad().make_ref<FileError>(
			this->path, "", FileErrorType::ACCESS_DENIED, FileOperation::WRITE_FILE
		)
	);

	usize bytes_written =
		fwrite(bytes.buf(), sizeof(uint8), bytes.len(), static_cast<FILE*>(this->fptr));
	if (bytes_written < bytes.len()) {
		return FileError::from_errno(this->path, "", FileOperation::WRITE_FILE);
	}
	return {};
}

bool tr::File::can_read()
{
	switch (this->mode) {
	case FileMode::READ_TEXT:
	case FileMode::READ_BINARY:
	case FileMode::READ_WRITE_TEXT:
	case FileMode::READ_WRITE_BINARY:
		return true;

	default:
		return false;
	}
}

bool tr::File::can_write()
{
	switch (this->mode) {
	case FileMode::WRITE_TEXT:
	case FileMode::WRITE_BINARY:
	case FileMode::READ_WRITE_TEXT:
	case FileMode::READ_WRITE_BINARY:
		return true;

	default:
		return false;
	}
}

tr::Result<void> tr::remove_file(tr::String path)
{
	FileError::reset_errors();

	int i = remove(path);
	if (i == -1) {
		return FileError::from_errno(path, "", FileOperation::REMOVE_FILE);
	}
	return {};
}

tr::Result<void> tr::move_file(tr::String from, tr::String to)
{
	FileError::reset_errors();

	// libc rename() is different on windows and posix
	// on posix it replaces the destination if it already exists
	// on windows it fails in that case
	if (tr::path_exists(to)) {
		return FileError(from, to, FileErrorType::FILE_EXISTS, FileOperation::MOVE_FILE);
	}

	int i = rename(from, to);
	if (i == -1) {
		return FileError::from_errno(from, to, FileOperation::MOVE_FILE);
	}
	return {};
}

[[deprecated("use tr::path_exists instead")]]
bool tr::file_exists(tr::String path)
{
	DWORD attr = GetFileAttributesW(from_trippin_to_win32_str(path));
	// NOLINTBEGIN(readability-implicit-bool-conversion)
	return attr != INVALID_FILE_ATTRIBUTES;
	// NOLINTEND(readability-implicit-bool-conversion)
}

bool tr::path_exists(tr::String path)
{
	DWORD attr = GetFileAttributesW(from_trippin_to_win32_str(path));
	// NOLINTBEGIN(readability-implicit-bool-conversion)
	return attr != INVALID_FILE_ATTRIBUTES;
	// NOLINTEND(readability-implicit-bool-conversion)
}

tr::Result<void> tr::create_dir(tr::String path)
{
	FileError::reset_errors();

	// it's recursive :)
	path = path.replace(tr::scratchpad(), '\\', '/');
	Array<String> dirs = path.split(tr::scratchpad(), '/');
	if (dirs.len() == 0) {
		tr::warn("couldn't create directory '%s', path is likely corrupted/invalid", *path);
		return {};
	}

	String full_dir = dirs[0];

	for (auto [i, dir] : dirs) {
		if (i > 0) {
			full_dir = tr::fmt(tr::scratchpad(), "%s/%s", *full_dir, *dir);
		}

		if (tr::path_exists(full_dir)) {
			// clang-format are you stupid
			TR_TRY_ASSIGN(
			bool is_file, tr::is_file(full_dir)
			);

			TR_TRY_ASSERT(
				!is_file, tr::scratchpad().make_ref<FileError>(
						  full_dir, "", FileErrorType::IS_NOT_DIRECTORY,
						  FileOperation::CREATE_DIR
					  )
			);
			continue;
		}

		if (_wmkdir(from_trippin_to_win32_str(full_dir)) == -1) {
			return FileError::from_win32(full_dir, "", FileOperation::CREATE_DIR);
		}
	}
	return {};
}

tr::Result<void> tr::remove_dir(tr::String path)
{
	FileError::reset_errors();

	// it's not my fault windows is garbage :(
	// NOLINTBEGIN(readability-implicit-bool-conversion)
	if (!RemoveDirectoryW(from_trippin_to_win32_str(path))) {
		return FileError::from_win32(path, "", FileOperation::REMOVE_DIR);
	}
	// NOLINTEND(readability-implicit-bool-conversion)
	return {};
}

tr::Result<tr::Array<tr::String>>
tr::list_dir(tr::Arena& arena, tr::String path, bool include_hidden)
{
	// this looks so horrible what the fuck is wrong with you bill gates
	WIN32_FIND_DATAW find_file_data;
	HANDLE hfind;

	hfind = FindFirstFileW(from_trippin_to_win32_str(path), &find_file_data);

	if (hfind == INVALID_HANDLE_VALUE) {
		return FileError::from_win32(path, "", FileOperation::LIST_DIR);
	}

	Array<String> entries(arena);

	// idk why it uses do-while i stole this
	do {
		if (wcscmp(find_file_data.cFileName, L".") == 0) {
			continue;
		}
		if (wcscmp(find_file_data.cFileName, L"..") == 0) {
			continue;
		}

		if (!include_hidden) {
			// NOLINTBEGIN(readability-implicit-bool-conversion)
			if (find_file_data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) {
				continue;
			}
			// NOLINTEND(readability-implicit-bool-conversion)
		}

		entries.add(from_win32_to_trippin_str(find_file_data.cFileName));
	} while (FindNextFileW(hfind, &find_file_data) != 0);

	FindClose(hfind);
	return entries;
}

tr::Result<bool> tr::is_file(tr::String path)
{
	DWORD attributes = GetFileAttributesW(from_trippin_to_win32_str(path));

	if (attributes == INVALID_FILE_ATTRIBUTES) {
		return FileError::from_win32(path, "", FileOperation::IS_FILE);
	}

	// NOLINTBEGIN(readability-implicit-bool-conversion)
	return !(attributes & FILE_ATTRIBUTE_DIRECTORY);
	// NOLINTEND(readability-implicit-bool-conversion)
}

void tr::_init_paths()
{
	// we're first getting it as utf-16 then converting it back to utf-8 just in case lmao
	WinStrMut exedir = static_cast<WinStrMut>(tr::core_arena.alloc(MAX_PATH * sizeof(wchar_t)));
	HMODULE hmodule = GetModuleHandle(nullptr);
	if (hmodule != nullptr) {
		DWORD len = GetModuleFileNameW(hmodule, exedir, MAX_PATH);
		if (len == 0) {
			tr::warn(
				"couldn't get executable directory, using relative paths for "
				"app://"
			);
			tr::exe_dir = ".";
		}
		else {
			if (len == MAX_PATH) {
				exedir[MAX_PATH] = '\0';
			}

			// utfma balls guys amirite
			tr::exe_dir = from_win32_to_trippin_str(exedir);
			tr::exe_dir = tr::exe_dir.directory(tr::core_arena);
		}
	}
	else {
		tr::warn("couldn't get executable directory, using relative paths for app://");
		tr::exe_dir = ".";
	}

	// msvc complains about getenv
	#ifdef TR_ONLY_MSVC
	auto* buf = static_cast<wchar_t*>(tr::scratchpad().alloc(MAX_PATH * sizeof(wchar_t)));
	_wdupenv_s(&buf, nullptr, L"APPDATA");
	tr::appdata_dir = from_win32_to_trippin_str(buf).duplicate(tr::core_arena);
	#else
	char* appdata = getenv("APPDATA");
	tr::appdata_dir = String(appdata).duplicate(tr::core_arena);
	#endif

	// normalize path separators
	// just in case
	tr::exe_dir = tr::exe_dir.replace(tr::core_arena, '\\', '/');
	tr::appdata_dir = tr::appdata_dir.replace(tr::core_arena, '\\', '/');
}

#else
/*
 * POSIX IMPLEMENTATION
 */

tr::Result<tr::File&> tr::File::open(tr::Arena& arena, tr::String path, tr::FileMode mode)
{
	FileError::reset_errors();

	// get mode
	String modefrfr;
	switch (mode) {
	case FileMode::READ_TEXT:
		modefrfr = "r";
		break;
	case FileMode::READ_BINARY:
		modefrfr = "rb";
		break;
	case FileMode::WRITE_TEXT:
		modefrfr = "w";
		break;
	case FileMode::WRITE_BINARY:
		modefrfr = "wb";
		break;
	case FileMode::READ_WRITE_TEXT:
		modefrfr = "r+";
		break;
	case FileMode::READ_WRITE_BINARY:
		modefrfr = "rb+";
		break;
	default:
		modefrfr = "";
		break;
	}

	File& file = arena.make_ref<File>();
	file.fptr = fopen(path, modefrfr);
	if (file.fptr == nullptr) {
		return FileError::from_errno(path, "", FileOperation::OPEN_FILE);
	}

	file.is_std = false;
	file.mode = mode;
	file.path = path.duplicate(arena);

	// get length :)))))))))
	fseek(static_cast<FILE*>(file.fptr), 0, SEEK_END);
	file.length = ftell(static_cast<FILE*>(file.fptr));
	::rewind(static_cast<FILE*>(file.fptr));

	return file;
}

void tr::File::close()
{
	FileError::reset_errors();

	// is_std exists so it doesn't close tr::std_out and company
	if (!this->is_std && this->fptr != nullptr) {
		fclose(static_cast<FILE*>(this->fptr));
	}
	this->fptr = nullptr;
}

tr::Result<int64> tr::File::position()
{
	FileError::reset_errors();

	int64 pos = ftell(static_cast<FILE*>(this->fptr));
	if (pos < 0) {
		return FileError::from_errno(this->path, "", FileOperation::GET_FILE_POSITION);
	}
	return pos;
}

tr::Result<int64> tr::File::len()
{
	FileError::reset_errors();
	return this->length;
}

tr::Result<bool> tr::File::eof()
{
	FileError::reset_errors();

	return feof(static_cast<FILE*>(this->fptr)) != 0;
}

tr::Result<void> tr::File::seek(int64 bytes, tr::SeekFrom from)
{
	FileError::reset_errors();

	int whence = SEEK_CUR;
	switch (from) {
	case SeekFrom::START:
		whence = SEEK_SET;
		break;
	case SeekFrom::CURRENT:
		whence = SEEK_CUR;
		break;
	case SeekFrom::END:
		whence = SEEK_END;
		break;
	}

	int i = fseek(static_cast<FILE*>(this->fptr), bytes, whence);
	if (i != 0) {
		return FileError::from_errno(this->path, "", FileOperation::SEEK_FILE);
	}
	return {};
}

tr::Result<void> tr::File::rewind()
{
	FileError::reset_errors();

	::rewind(static_cast<FILE*>(this->fptr));
	if (errno != 0) {
		return FileError::from_errno(this->path, "", FileOperation::REWIND_FILE);
	}
	return {};
}

tr::Result<int64> tr::File::read_bytes(void* out, int64 size, int64 items)
{
	FileError::reset_errors();
	TR_TRY_ASSERT(
		out != nullptr, tr::scratchpad().make_ref<StringError>(
					"you dumbass it's supposed to go somewhere if you don't "
					"want to use it use File::seek() dumbass"
				)
	);
	TR_TRY_ASSERT(
		this->can_read(),
		tr::scratchpad().make_ref<FileError>(
			this->path, "", FileErrorType::ACCESS_DENIED, FileOperation::READ_FILE
		)
	);

	// TODO 32-bit won't be happy about this
	usize bytes =
		fread(out, static_cast<usize>(size), static_cast<usize>(items),
		      static_cast<FILE*>(this->fptr));

	if (errno != 0) {
		return FileError::from_errno(this->path, "", FileOperation::READ_FILE);
	}
	return static_cast<int64>(bytes);
}

tr::Result<void> tr::File::flush()
{
	FileError::reset_errors();

	int i = fflush(static_cast<FILE*>(this->fptr));
	if (i == EOF) {
		return FileError::from_errno(this->path, "", FileOperation::FLUSH_FILE);
	}
	return {};
}

tr::Result<void> tr::File::write_bytes(Array<uint8> bytes)
{
	FileError::reset_errors();
	TR_TRY_ASSERT(
		this->can_write(),
		tr::scratchpad().make_ref<FileError>(
			this->path, "", FileErrorType::ACCESS_DENIED, FileOperation::WRITE_FILE
		)
	);

	usize bytes_written =
		fwrite(bytes.buf(), sizeof(uint8), bytes.len(), static_cast<FILE*>(this->fptr));
	if (bytes_written < bytes.len()) {
		return FileError::from_errno(this->path, "", FileOperation::WRITE_FILE);
	}
	return {};
}

bool tr::File::can_read()
{
	switch (this->mode) {
	case FileMode::READ_TEXT:
	case FileMode::READ_BINARY:
	case FileMode::READ_WRITE_TEXT:
	case FileMode::READ_WRITE_BINARY:
		return true;

	default:
		return false;
		return false;
	}
}

bool tr::File::can_write()
{
	switch (this->mode) {
	case FileMode::WRITE_TEXT:
	case FileMode::WRITE_BINARY:
	case FileMode::READ_WRITE_TEXT:
	case FileMode::READ_WRITE_BINARY:
		return true;

	default:
		return false;
	}
}

tr::Result<void> tr::remove_file(tr::String path)
{
	FileError::reset_errors();

	int i = remove(path);
	if (i == -1) {
		return FileError::from_errno(path, "", FileOperation::REMOVE_FILE);
	}
	return {};
}

tr::Result<void> tr::move_file(tr::String from, tr::String to)
{
	FileError::reset_errors();

	// libc rename() is different on windows and posix
	// on posix it replaces the destination if it already exists
	// on windows it fails in that case
	if (tr::path_exists(to)) {
		return FileError(from, to, FileErrorType::FILE_EXISTS, FileOperation::MOVE_FILE);
	}

	int i = rename(from, to);
	if (i == -1) {
		return FileError::from_errno(from, to, FileOperation::MOVE_FILE);
	}
	return {};
}

[[deprecated("use tr::path_exists instead")]]
bool tr::file_exists(tr::String path)
{
	FileError::reset_errors();

	// we could just fopen(path, "r") then check if that's null, but then it would return false
	// on permission errors, even though it does in fact exist
	struct stat buffer = {};
	return stat(path, &buffer) == 0;
}

bool tr::path_exists(tr::String path)
{
	FileError::reset_errors();

	// we could just fopen(path, "r") then check if that's null, but then it would return false
	// on permission errors, even though it does in fact exist
	struct stat buffer = {};
	return stat(path, &buffer) == 0;
}

tr::Result<void> tr::create_dir(tr::String path)
{
	FileError::reset_errors();

	// it's recursive :)
	Array<String> dirs = path.split(tr::scratchpad(), '/');
	if (dirs.len() == 0) {
		tr::warn("couldn't create directory '%s', path is likely corrupted/invalid", *path);
		return {};
	}

	// help
	String full_dir;
	if (path.starts_with("/")) {
		full_dir = tr::fmt(tr::scratchpad(), "/%s", *dirs[0]);
	}
	else {
		full_dir = dirs[0];
	}

	for (auto [i, dir] : dirs) {
		if (i > 0) {
			full_dir = tr::fmt(tr::scratchpad(), "%s/%s", *full_dir, *dir);
		}

		if (tr::path_exists(full_dir)) {
			TR_TRY_ASSIGN(bool is_file, tr::is_file(full_dir));
			TR_TRY_ASSERT(
				!is_file, tr::scratchpad().make_ref<FileError>(
						  full_dir, "", FileErrorType::IS_NOT_DIRECTORY,
						  FileOperation::CREATE_DIR
					  )
			);
			continue;
		}

		if (mkdir(full_dir, 0755) == -1) {
			return FileError::from_errno(full_dir, "", FileOperation::CREATE_DIR);
		}
	}
	return {};
}

tr::Result<void> tr::remove_dir(tr::String path)
{
	FileError::reset_errors();

	if (rmdir(path) != 0) {
		return FileError::from_errno(path, "", FileOperation::REMOVE_DIR);
	}
	return {};
}

tr::Result<tr::Array<tr::String>>
tr::list_dir(tr::Arena& arena, tr::String path, bool include_hidden)
{
	FileError::reset_errors();

	DIR* dir = opendir(path);
	if (dir == nullptr) {
		return FileError::from_errno(path, "", FileOperation::LIST_DIR);
	}

	Array<String> entries(arena);
	struct dirent* entry;

	while ((entry = readdir(dir)) != nullptr) {
		if (String(entry->d_name) == ".") {
			continue;
		}
		if (String(entry->d_name) == "..") {
			continue;
		}

		if (!include_hidden) {
			if (String(entry->d_name).starts_with(".")) {
				continue;
			}
		}

		entries.add(String(arena, entry->d_name, strlen(entry->d_name)));
	}

	return entries;
}

tr::Result<bool> tr::is_file(tr::String path)
{
	FileError::reset_errors();

	struct stat statma = {};
	if (stat(path, &statma) != 0) {
		return FileError::from_errno(path, "", FileOperation::IS_FILE);
	}

	// TODO there's other types but they're similar to files so i'm counting all of them as
	// files too
	if (S_ISDIR(statma.st_mode)) {
		return false;
	}
	return true;
}

void tr::_init_paths()
{
	// TODO macOS exists
	// TODO bsd exists
	tr::exe_dir = String(tr::core_arena, PATH_MAX);
	ssize_t len = readlink("/proc/self/exe", exe_dir, exe_dir.len());
	if (len == -1) {
		tr::warn("couldn't get executable directory, using relative paths for app://");
		tr::exe_dir = ".";
	}
	else {
		tr::exe_dir[static_cast<usize>(len)] = '\0';
		tr::exe_dir = tr::exe_dir.directory(tr::core_arena);
	}

	char* home = getenv("HOME");
	tr::appdata_dir = tr::fmt(tr::core_arena, "%s/.local/share", home);
}

#endif
