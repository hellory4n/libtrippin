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

#include "trippin/iofs.h"

// TODO macOS exists
// though macOS should be easier as it supports posix
#ifdef _WIN32
	#include <cstdio>
	// windows and its consequences have been a disaster for the human race
	#include <direct.h>

	#include "trippin/antiwindows.h"
#else
	#include <cerrno>
	#include <cstdio>
	#include <cstdlib>

	#include <dirent.h>
	#include <sys/stat.h>
	#include <sys/types.h>
	#include <unistd.h>
#endif

#include "trippin/common.h"
#include "trippin/error.h"
#include "trippin/log.h"
#include "trippin/memory.h"
#include "trippin/string.h"
/* clang-format off */
#include "trippin/bits/state.h"
/* clang-format on */

namespace tr {

String exe_dir;
String appdata_dir;

String app_dir;
String user_dir;

}

tr::Result<tr::String> tr::Reader::read_string(tr::Arena& arena, int64 length)
{
	StringBuilder str{arena, static_cast<usize>(length)};
	int64 read = TR_TRY(this->read_bytes(str.buf(), sizeof(char), length));

	if (read != int64(length)) {
		return {ERROR_EXPECTED_MORE_BYTES, length, read};
	}
	return static_cast<String>(str);
}

tr::Result<tr::String> tr::Reader::read_line(Arena& arena)
{
	ScratchArena scratch{};
	TR_DEFER(scratch.free());
	Array<char> linema{scratch, 0};

	while (true) {
		char byte = '\0';
		int64 read = TR_TRY(this->read_bytes(&byte, sizeof(char), 1));

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
			read = TR_TRY(this->read_bytes(&next_byte, sizeof(char), 1));

			// eof still counts
			if (read == 0 || next_byte == '\n') {
				break;
			}
		}

		// normal character
		linema.add(byte);
	}

	if (linema.len() == 0) {
		return String{""};
	}
	return String{arena, linema.buf(), linema.len() + 1};
}

tr::Result<tr::Array<uint8>> tr::Reader::read_all_bytes(tr::Arena& arena)
{
	int64 length = TR_TRY(this->len());

	Array<uint8> man{arena, static_cast<usize>(length)};
	TR_TRY(this->read_bytes(man.buf(), sizeof(uint8), length));
	return man;
}

tr::Result<tr::String> tr::Reader::read_all_text(tr::Arena& arena)
{
	int64 length = TR_TRY(this->len());

	StringBuilder man{arena, static_cast<usize>(length)};
	TR_TRY(this->read_bytes(man.buf(), sizeof(char), length));
	return static_cast<String>(man);
}

tr::Result<void> tr::Writer::write_string(tr::String str)
{
	Array<uint8> manfuckyou{reinterpret_cast<const uint8*>(str.buf()), str.len()};
	return this->write_bytes(manfuckyou);
}

tr::Result<void> tr::Writer::printf(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	ScratchArena scratch{};
	TR_DEFER(scratch.free());
	String str = tr::fmt_args(scratch, fmt, args);
	va_end(args);

	return this->write_string(str);
}

tr::Result<void> tr::Writer::print_args(const char* fmt, va_list arg)
{
	ScratchArena scratch{};
	TR_DEFER(scratch.free());
	String str = tr::fmt_args(scratch, fmt, arg);
	return write_string(str);
}

tr::Result<void> tr::Writer::print(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	Result<void> man = print_args(fmt, args);
	va_end(args);
	return man;
}

tr::Result<void> tr::Writer::println(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	TR_TRY(print_args(fmt, args));
	va_end(args);
	return write_string("\n");
}

tr::String tr::path(tr::Arena& arena, tr::String path)
{
	ScratchArena scratch{};
	TR_DEFER(scratch.free());

	if (path.starts_with("app://")) {
		String pathfrfr = path.substr(scratch, sizeof("app://") - 1, path.len());
		return tr::fmt(
			arena, "%s/%s/%s", tr::exe_dir.buf(), tr::app_dir.buf(), pathfrfr.buf()
		);
	}

	if (path.starts_with("user://")) {
		String pathfrfr = path.substr(scratch, sizeof("user://") - 1, path.len());
		return tr::fmt(
			arena, "%s/%s/%s", tr::appdata_dir.buf(), tr::user_dir.buf(), pathfrfr.buf()
		);
	}

	return path;
}

void tr::set_paths(tr::String appdir, tr::String userdir)
{
	tr::app_dir = appdir.duplicate(_tr::core_arena());
	tr::user_dir = userdir.duplicate(_tr::core_arena());
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
static WinStrConst from_trippin_to_win32_str(tr::Arena& arena, tr::String str)
{
	// conveniently microsoft knows this is torture and gives a function for this exact purpose
	int size = MultiByteToWideChar(CP_UTF8, 0, str.buf(), -1, nullptr, 0);
	// TODO idk if it can go negative but the headers don't have documentation, and the search
	// online button uses bing
	// let's just say, that this table right here, is bill gates!
	// *smashes table* YEAHHHHHHHHHHHHHH https://www.youtube.com/watch?v=WGFLPbpdMS8
	TR_ASSERT_MSG(size != 0, "blame it on windows");

	WinStrMut new_str = arena.alloc<WinStrMut>(static_cast<usize>(size + 1) * sizeof(wchar_t));
	int result = MultiByteToWideChar(CP_UTF8, 0, str.buf(), -1, new_str, size);
	TR_ASSERT_MSG(result != 0, "blame it on windows");
	return new_str;
}

static tr::String from_win32_to_trippin_str(tr::Arena& arena, WinStrConst str)
{
	int size = WideCharToMultiByte(CP_UTF8, 0, str, -1, nullptr, 0, nullptr, nullptr);
	TR_ASSERT_MSG(size != 0, "blame it on windows");

	tr::StringBuilder new_str{arena, static_cast<usize>(size)};
	int result =
		WideCharToMultiByte(CP_UTF8, 0, str, -1, new_str.buf(), size, nullptr, nullptr);
	TR_ASSERT_MSG(result != 0, "blame it on windows");
	return new_str;
}

tr::Result<tr::File> tr::File::open(tr::Arena& arena, tr::String path, FileMode mode)
{
	ScratchArena scratch{};
	TR_DEFER(scratch.free());
	path = tr::path(scratch, path);
	tr::_reset_os_errors();

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

	File file{};
	file.fptr = _wfopen(from_trippin_to_win32_str(scratch, path), modefrfr);
	if (errno != 0) {
		return {_trippin_error_from_errno(), FileOperation::OPEN_FILE, path, ""};
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
	tr::_reset_os_errors();

	// is_std exists so it doesn't close tr::std_out and company
	if (!is_std && fptr != nullptr) {
		fclose(static_cast<FILE*>(fptr));
	}
	fptr = nullptr;
}

tr::Result<int64> tr::File::position()
{
	tr::_reset_os_errors();

	int64 pos = _ftelli64(static_cast<FILE*>(this->fptr));
	if (pos < 0) {
		return {_trippin_error_from_errno(), FileOperation::GET_FILE_POSITION, path, ""};
	}
	return pos;
}

tr::Result<int64> tr::File::len()
{
	tr::_reset_os_errors();
	return length;
}

tr::Result<bool> tr::File::eof()
{
	tr::_reset_os_errors();
	return feof(static_cast<FILE*>(this->fptr)) != 0;
}

tr::Result<void> tr::File::seek(int64 bytes, tr::SeekFrom from)
{
	tr::_reset_os_errors();

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

	int i = _fseeki64(static_cast<FILE*>(fptr), bytes, whence);
	if (i != 0) {
		return {_trippin_error_from_errno(), FileOperation::SEEK_FILE, path, ""};
	}
	return {};
}

tr::Result<void> tr::File::rewind()
{
	tr::_reset_os_errors();

	::rewind(static_cast<FILE*>(fptr));
	if (errno != 0) {
		return {_trippin_error_from_errno(), FileOperation::REWIND_FILE, path, ""};
	}
	return {};
}

tr::Result<int64> tr::File::read_bytes(void* out, int64 size, int64 items)
{
	tr::_reset_os_errors();
	TR_ASSERT(out != nullptr);
	TR_TRY_ASSERT(can_read(), {ERROR_ACCESS_DENIED, FileOperation::READ_FILE, path, ""});

	usize bytes =
		fread(out, static_cast<usize>(size), static_cast<usize>(items),
		      static_cast<FILE*>(fptr));
	if (errno != 0) {
		return {_trippin_error_from_errno(), path, "", FileOperation::READ_FILE};
	}
	return static_cast<int64>(bytes);
}

tr::Result<void> tr::File::flush()
{
	tr::_reset_os_errors();

	int i = fflush(static_cast<FILE*>(fptr));
	if (i == EOF) {
		return {_trippin_error_from_errno(), FileOperation::FLUSH_FILE, path, ""};
	}
	return {};
}

tr::Result<void> tr::File::write_bytes(Array<const uint8> bytes)
{
	tr::_reset_os_errors();
	TR_TRY_ASSERT(can_write(), {ERROR_ACCESS_DENIED, FileOperation::WRITE_FILE, path, ""});

	usize bytes_written =
		fwrite(bytes.buf(), sizeof(uint8), bytes.len(), static_cast<FILE*>(fptr));
	if (bytes_written < bytes.len()) {
		return {_trippin_error_from_errno(), FileOperation::WRITE_FILE, path, ""};
	}
	return {};
}

tr::Result<void> tr::File::print_args(const char* fmt, va_list arg)
{
	tr::_reset_os_errors();
	TR_TRY_ASSERT(can_write(), {ERROR_ACCESS_DENIED, FileOperation::WRITE_FILE, path, ""});

	vfprintf(static_cast<FILE*>(fptr), fmt, arg);
	if (errno != 0) {
		return {tr::_trippin_error_from_errno(), FileOperation::WRITE_FILE, this->path, ""};
	}
	return {};
}

bool tr::File::can_read()
{
	switch (mode) {
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
	switch (mode) {
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
	ScratchArena scratch{};
	TR_DEFER(scratch.free());
	path = tr::path(scratch, path);
	tr::_reset_os_errors();

	int i = _wremove(from_trippin_to_win32_str(scratch, path));
	if (i == -1) {
		return {_trippin_error_from_errno(), FileOperation::REMOVE_FILE, path, ""};
	}
	return {};
}

tr::Result<void> tr::move_file(tr::String from, tr::String to)
{
	ScratchArena scratch{};
	TR_DEFER(scratch.free());
	from = tr::path(scratch, from);
	to = tr::path(scratch, to);
	tr::_reset_os_errors();

	// libc rename() is different on windows and posix
	// on posix it replaces the destination if it already exists
	// on windows it fails in that case
	if (tr::path_exists(to)) {
		return {ERROR_FILE_EXISTS, FileOperation::MOVE_FILE, from, to};
	}

	int i = _wrename(
		from_trippin_to_win32_str(scratch, from), from_trippin_to_win32_str(scratch, to)
	);
	if (i == -1) {
		return {_trippin_error_from_errno(), FileOperation::MOVE_FILE, from, to};
	}
	return {};
}

[[deprecated("use tr::path_exists instead")]]
bool tr::file_exists(tr::String path)
{
	ScratchArena scratch{};
	TR_DEFER(scratch.free());
	path = tr::path(scratch, path);
	DWORD attr = GetFileAttributesW(from_trippin_to_win32_str(scratch, path));
	return attr != INVALID_FILE_ATTRIBUTES;
}

bool tr::path_exists(tr::String path)
{
	ScratchArena scratch{};
	TR_DEFER(scratch.free());
	path = tr::path(scratch, path);
	DWORD attr = GetFileAttributesW(from_trippin_to_win32_str(scratch, path));
	return attr != INVALID_FILE_ATTRIBUTES;
}

tr::Result<void> tr::create_dir(tr::String path)
{
	ScratchArena scratch{};
	TR_DEFER(scratch.free());
	path = tr::path(scratch, path);
	tr::_reset_os_errors();

	// it's recursive :)
	path = path.replace(scratch, '\\', '/');
	Array<String> dirs = path.split(scratch, '/');
	if (dirs.len() == 0) {
		tr::warn("couldn't create directory '%s', path is likely corrupted/invalid", *path);
		return {};
	}

	String full_dir = dirs[0];

	for (auto [i, dir] : dirs) {
		if (i > 0) {
			full_dir = tr::tmp_fmt("%s/%s", *full_dir, *dir);
		}

		if (tr::path_exists(full_dir)) {
			bool is_file = TR_TRY(tr::is_file(full_dir));

			TR_TRY_ASSERT(
				!is_file, {
						  ERROR_IS_NOT_DIRECTORY,
						  FileOperation::CREATE_DIR,
						  full_dir,
						  "",
					  }
			);
			continue;
		}

		if (_wmkdir(from_trippin_to_win32_str(scratch, full_dir)) == -1) {
			return {_trippin_error_from_errno(), FileOperation::CREATE_DIR, full_dir,
				""};
		}
	}
	return {};
}

tr::Result<void> tr::remove_dir(tr::String path)
{
	ScratchArena scratch{};
	TR_DEFER(scratch.free());
	path = tr::path(scratch, path);
	tr::_reset_os_errors();

	if (!RemoveDirectoryW(from_trippin_to_win32_str(scratch, path))) {
		return {_trippin_error_from_win32(), FileOperation::REMOVE_DIR, path, ""};
	}
	return {};
}

tr::Result<tr::Array<tr::String>>
tr::list_dir(tr::Arena& arena, tr::String path, bool include_hidden)
{
	ScratchArena scratch{};
	TR_DEFER(scratch.free());
	path = tr::path(scratch, path);
	// this looks so horrible what the fuck is wrong with you bill gates
	WIN32_FIND_DATAW find_file_data;
	HANDLE hfind;

	hfind = FindFirstFileW(from_trippin_to_win32_str(scratch, path), &find_file_data);

	if (hfind == INVALID_HANDLE_VALUE) {
		return {_trippin_error_from_win32(), FileOperation::LIST_DIR, path, ""};
	}

	Array<String> entries{arena};

	// idk why it uses do-while i stole this
	do {
		if (wcscmp(find_file_data.cFileName, L".") == 0) {
			continue;
		}
		if (wcscmp(find_file_data.cFileName, L"..") == 0) {
			continue;
		}

		if (!include_hidden) {
			if (find_file_data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) {
				continue;
			}
		}

		entries.add(from_win32_to_trippin_str(scratch, find_file_data.cFileName));
	} while (FindNextFileW(hfind, &find_file_data) != 0);

	FindClose(hfind);
	return entries;
}

tr::Result<bool> tr::is_file(tr::String path)
{
	ScratchArena scratch{};
	TR_DEFER(scratch.free());
	path = tr::path(scratch, path);
	DWORD attributes = GetFileAttributesW(from_trippin_to_win32_str(scratch, path));

	if (attributes == INVALID_FILE_ATTRIBUTES) {
		return {_trippin_error_from_win32(), FileOperation::IS_FILE, path, ""};
	}

	return !(attributes & FILE_ATTRIBUTE_DIRECTORY);
}

void tr::_init_paths()
{
	ScratchArena scratch{};
	TR_DEFER(scratch.free());

	// we're first getting it as utf-16 then converting it back to utf-8 just in case lmao
	WinStrMut exedir = _tr::core_arena().alloc<WinStrMut>(MAX_PATH * sizeof(wchar_t));
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
			tr::exe_dir = from_win32_to_trippin_str(scratch, exedir);
			tr::exe_dir = tr::exe_dir.directory(_tr::core_arena());
		}
	}
	else {
		tr::warn("couldn't get executable directory, using relative paths for app://");
		tr::exe_dir = ".";
	}

	wchar_t* buf = scratch.alloc<wchar_t*>(MAX_PATH * sizeof(wchar_t));
	_wdupenv_s(&buf, nullptr, L"APPDATA");
	tr::appdata_dir = from_win32_to_trippin_str(scratch, buf).duplicate(_tr::core_arena());

	// normalize path separators
	// just in case
	tr::exe_dir = tr::exe_dir.replace(_tr::core_arena(), '\\', '/');
	tr::appdata_dir = tr::appdata_dir.replace(_tr::core_arena(), '\\', '/');
}

#else
/*
 * POSIX IMPLEMENTATION
 */

tr::Result<tr::File> tr::File::open(tr::Arena& arena, tr::String path, tr::FileMode mode)
{
	ScratchArena scratch{};
	TR_DEFER(scratch.free());
	path = tr::path(scratch, path);
	tr::_reset_os_errors();

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

	File file{};
	file.fptr = fopen(*path, *modefrfr);
	if (file.fptr == nullptr) {
		return {tr::_trippin_error_from_errno(), FileOperation::OPEN_FILE, path, ""};
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
	tr::_reset_os_errors();

	// is_std exists so it doesn't close tr::std_out and company
	if (!this->is_std && this->fptr != nullptr) {
		fclose(static_cast<FILE*>(this->fptr));
	}
	this->fptr = nullptr;
}

tr::Result<int64> tr::File::position()
{
	tr::_reset_os_errors();

	int64 pos = ftell(static_cast<FILE*>(this->fptr));
	if (pos < 0) {
		return {tr::_trippin_error_from_errno(), FileOperation::GET_FILE_POSITION,
			this->path, ""};
	}
	return pos;
}

tr::Result<int64> tr::File::len()
{
	return length;
}

tr::Result<bool> tr::File::eof()
{
	tr::_reset_os_errors();
	return feof(static_cast<FILE*>(this->fptr)) != 0;
}

tr::Result<void> tr::File::seek(int64 bytes, tr::SeekFrom from)
{
	tr::_reset_os_errors();

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
		return {tr::_trippin_error_from_errno(), FileOperation::SEEK_FILE, this->path, ""};
	}
	return {};
}

tr::Result<void> tr::File::rewind()
{
	tr::_reset_os_errors();

	::rewind(static_cast<FILE*>(this->fptr));
	if (errno != 0) {
		return {tr::_trippin_error_from_errno(), FileOperation::REWIND_FILE, this->path,
			""};
	}
	return {};
}

tr::Result<int64> tr::File::read_bytes(void* out, int64 size, int64 items)
{
	tr::_reset_os_errors();
	TR_ASSERT(out != nullptr);
	TR_TRY_ASSERT(
		this->can_read(), {ERROR_ACCESS_DENIED, FileOperation::READ_FILE, this->path, ""}
	);

	// TODO 32-bit won't be happy about this
	usize bytes =
		fread(out, static_cast<usize>(size), static_cast<usize>(items),
		      static_cast<FILE*>(this->fptr));

	if (errno != 0) {
		return {tr::_trippin_error_from_errno(), FileOperation::READ_FILE, this->path, ""};
	}
	return static_cast<int64>(bytes);
}

tr::Result<void> tr::File::flush()
{
	tr::_reset_os_errors();

	int i = fflush(static_cast<FILE*>(this->fptr));
	if (i == EOF) {
		return {tr::_trippin_error_from_errno(), FileOperation::FLUSH_FILE, this->path, ""};
	}
	return {};
}

tr::Result<void> tr::File::write_bytes(Array<const byte> bytes)
{
	tr::_reset_os_errors();
	TR_TRY_ASSERT(
		this->can_write(), {ERROR_ACCESS_DENIED, FileOperation::WRITE_FILE, this->path, ""}
	);

	usize bytes_written =
		fwrite(bytes.buf(), sizeof(byte), bytes.len(), static_cast<FILE*>(this->fptr));
	if (bytes_written < bytes.len()) {
		return {tr::_trippin_error_from_errno(), FileOperation::WRITE_FILE, this->path, ""};
	}
	return {};
}

tr::Result<void> tr::File::print_args(const char* fmt, va_list arg)
{
	tr::_reset_os_errors();
	TR_TRY_ASSERT(can_write(), {ERROR_ACCESS_DENIED, FileOperation::WRITE_FILE, path, ""});

	vfprintf(static_cast<FILE*>(fptr), fmt, arg);
	if (errno != 0) {
		return {tr::_trippin_error_from_errno(), FileOperation::WRITE_FILE, this->path, ""};
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
	ScratchArena scratch{};
	TR_DEFER(scratch.free());
	path = tr::path(scratch, path);
	tr::_reset_os_errors();

	int i = remove(*path);
	if (i == -1) {
		return {tr::_trippin_error_from_errno(), FileOperation::REMOVE_FILE, path, ""};
	}
	return {};
}

tr::Result<void> tr::move_file(tr::String from, tr::String to)
{
	ScratchArena scratch{};
	TR_DEFER(scratch.free());
	from = tr::path(scratch, from);
	to = tr::path(scratch, to);
	tr::_reset_os_errors();

	// libc rename() is different on windows and posix
	// on posix it replaces the destination if it already exists
	// on windows it fails in that case
	if (tr::path_exists(to)) {
		return {tr::_trippin_error_from_errno(), FileOperation::MOVE_FILE, from, to};
	}

	int i = rename(*from, *to);
	if (i == -1) {
		return {tr::_trippin_error_from_errno(), FileOperation::MOVE_FILE, from, to};
	}
	return {};
}

[[deprecated("use tr::path_exists instead")]]
bool tr::file_exists(tr::String path)
{
	tr::_reset_os_errors();

	// we could just fopen(path, "r") then check if that's null, but then it would
	// return false on permission errors, even though it does in fact exist
	struct stat buffer = {};
	return stat(*path, &buffer) == 0;
}

bool tr::path_exists(tr::String path)
{
	ScratchArena scratch{};
	TR_DEFER(scratch.free());
	path = tr::path(scratch, path);
	tr::_reset_os_errors();

	// we could just fopen(path, "r") then check if that's null, but then it would
	// return false on permission errors, even though it does in fact exist
	struct stat buffer = {};
	return stat(*path, &buffer) == 0;
}

tr::Result<void> tr::create_dir(tr::String path)
{
	ScratchArena scratch{};
	TR_DEFER(scratch.free());
	path = tr::path(scratch, path);
	tr::_reset_os_errors();

	// it's recursive :)
	Array<String> dirs = path.split(scratch, '/');
	if (dirs.len() == 0) {
		tr::warn("couldn't create directory '%s', path is likely corrupted/invalid", *path);
		return {};
	}

	// help
	String full_dir;
	if (path.starts_with("/")) {
		full_dir = tr::fmt(scratch, "/%s", *dirs[0]);
	}
	else {
		full_dir = dirs[0];
	}

	for (auto [i, dir] : dirs) {
		if (i > 0) {
			full_dir = tr::fmt(scratch, "%s/%s", *full_dir, *dir);
		}

		if (tr::path_exists(full_dir)) {
			bool is_file = TR_TRY(tr::is_file(full_dir));
			TR_TRY_ASSERT(
				!is_file, {
						  ERROR_IS_NOT_DIRECTORY,
						  FileOperation::CREATE_DIR,
						  full_dir,
						  "",
					  }
			);
			continue;
		}

		if (mkdir(*full_dir, 0755) == -1) {
			return {tr::_trippin_error_from_errno(), FileOperation::CREATE_DIR,
				full_dir, ""};
		}
	}
	return {};
}

tr::Result<void> tr::remove_dir(tr::String path)
{
	ScratchArena scratch{};
	TR_DEFER(scratch.free());
	path = tr::path(scratch, path);
	tr::_reset_os_errors();

	if (rmdir(*path) != 0) {
		return {tr::_trippin_error_from_errno(), FileOperation::REMOVE_DIR, path, ""};
	}
	return {};
}

tr::Result<tr::Array<tr::String>>
tr::list_dir(tr::Arena& arena, tr::String path, bool include_hidden)
{
	// FIXME this might be broken for some fucking reason
	ScratchArena scratch{};
	TR_DEFER(scratch.free());
	path = tr::path(scratch, path);
	tr::_reset_os_errors();

	DIR* dir = opendir(*path);
	if (dir == nullptr) {
		return {tr::_trippin_error_from_errno(), FileOperation::LIST_DIR, path, ""};
	}

	Array<String> entries{arena};
	struct dirent* entry;

	while ((entry = readdir(dir)) != nullptr) {
		if (String{entry->d_name} == ".") {
			continue;
		}
		if (String{entry->d_name} == "..") {
			continue;
		}

		if (!include_hidden) {
			if (String{entry->d_name}.starts_with(".")) {
				continue;
			}
		}

		entries.add(String{arena, entry->d_name});
	}

	return entries;
}

tr::Result<bool> tr::is_file(tr::String path)
{
	ScratchArena scratch{};
	TR_DEFER(scratch.free());
	path = tr::path(scratch, path);
	tr::_reset_os_errors();

	struct stat statma = {};
	if (stat(*path, &statma) != 0) {
		return {tr::_trippin_error_from_errno(), FileOperation::IS_FILE, path, ""};
	}

	// TODO there's other types but they're similar to files so i'm counting all of them
	// as files too
	if (S_ISDIR(statma.st_mode)) {
		return false;
	}
	return true;
}

void tr::_init_paths()
{
	// TODO macOS exists
	// TODO bsd exists
	StringBuilder exedir{_tr::core_arena(), PATH_MAX - 1};
	isize len = readlink("/proc/self/exe", *exedir, exedir.len());
	if (len < 1) {
		tr::warn("couldn't get executable directory, using relative paths for app://");
		tr::exe_dir = ".";
	}
	else {
		exedir[static_cast<usize>(len)] = '\0';
		// FIXME this copies the data twice for no reason, when it could just go
		// through StringBuilder directly
		exedir = {
			_tr::core_arena(),
			String{*exedir, static_cast<usize>(len)}
                        .directory(_tr::core_arena())
		};
	}

	tr::exe_dir = exedir;
	char* home = getenv("HOME");
	tr::appdata_dir = tr::fmt(_tr::core_arena(), "%s/.local/share", home);
}

#endif
