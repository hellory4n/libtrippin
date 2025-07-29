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

// :(
// TODO macOS exists
// though macOS should be easier as it supports posix
#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define NOSERVICE
	#define NOMCX
	#define NOIME
	#define NOMINMAX
	#include <windows.h>

	// conflicts :D
	#define WIN32_ERROR ERROR
	#undef ERROR

	#include <stdio.h>
#else
	#include <stdlib.h>
	#include <stdio.h>
	#include <sys/stat.h>
	#include <sys/types.h>
	#include <dirent.h>
	#include <unistd.h>
	#include <errno.h>
#endif

#include "trippin/log.h"
#include "trippin/iofs.h"

namespace tr {
	extern Arena core_arena;

	String exe_dir;
	String appdata_dir;

	String app_dir;
	String user_dir;
}

// TODO this was written before TR_TRY and all that crap
// so like, use that?

tr::Result<tr::String, const tr::Error&> tr::Reader::read_string(tr::Arena& arena, int64 length)
{
	String str(arena, static_cast<usize>(length));
	TR_TRY_ASSIGN(int64 read, this->read_bytes(str.buf(), sizeof(char), length));

	if (read != int64(length)) {
		return tr::scratchpad().make<StringError>("expected %zu bytes, got %li (might be EOF)", length, read);
	}
	else return str;
}

tr::Result<tr::String, const tr::Error&> tr::Reader::read_line(Arena& arena)
{
	Array<char> linema(tr::scratchpad(), 0);

	while (true) {
		char byte = '\0';
		TR_TRY_ASSIGN(int64 read, this->read_bytes(&byte, sizeof(char), 1));

		// eof? idfk man
		if (read == 0) break;

		// i love unixcxfukc
		if (byte == '\n') break;

		// windows :(
		if (byte == '\r') {
			char next_byte = '\0';
			TR_TRY_ASSIGN(read, this->read_bytes(&next_byte, sizeof(char), 1));

			// eof still counts
			if (read == 0 || next_byte == '\n') break;
		}

		// normal character
		linema.add(byte);
	}

	if (linema.len() == 0) return String("");
	else return String(arena, linema.buf(), linema.len() + 1);
}

tr::Result<tr::Array<uint8>, const tr::Error&> tr::Reader::read_all_bytes(tr::Arena& arena)
{
	TR_TRY_ASSIGN(int64 length, this->len());

	Array<uint8> man(arena, static_cast<usize>(length));
	TR_TRY(this->read_bytes(man.buf(), sizeof(uint8), length));
	return man;
}

tr::Result<tr::String, const tr::Error&> tr::Reader::read_all_text(tr::Arena& arena)
{
	TR_TRY_ASSIGN(int64 length, this->len());

	String man(arena, static_cast<usize>(length));
	TR_TRY(this->read_bytes(man.buf(), sizeof(char), length));
	return man;
}

tr::Result<void, const tr::Error&> tr::Writer::write_string(tr::String str)
{
	Array<uint8> manfuckyou(reinterpret_cast<uint8*>(str.buf()), str.len());
	return this->write_bytes(manfuckyou);
}

tr::Result<void, const tr::Error&> tr::Writer::printf(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	String str = tr::fmt_args(tr::scratchpad(), fmt, args);
	va_end(args);

	return this->write_string(str);
}

tr::Result<void, const tr::Error&> tr::Writer::println(const char* fmt, ...)
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
		String pathfrfr = path.substr(tr::scratchpad(), sizeof("app://")-1, path.len());
		return tr::fmt(arena, "%s/%s/%s", tr::exe_dir.buf(), tr::app_dir.buf(), pathfrfr.buf());
	}
	else if (path.starts_with("user://")) {
		String pathfrfr = path.substr(tr::scratchpad(), sizeof("user://")-1, path.len());
		return tr::fmt(arena, "%s/%s/%s", tr::appdata_dir.buf(), tr::user_dir.buf(), pathfrfr.buf());
	}
	else {
		return path.duplicate(arena);
	}
}

void tr::set_paths(tr::String appdir, tr::String userdir)
{
	tr::app_dir = appdir.duplicate(tr::core_arena);
	tr::user_dir = userdir.duplicate(tr::core_arena);
}

#ifdef _WIN32
/*
 * WINDOWS IMPLEMENTATION
 */

// TODO use only windows APIs (massive pain in the ass)

// you know how hard it is to remember LPCWSTR???? and visual studio takes a week to show the autocomplete
using WinStrConst = LPCWSTR;
using WinStrMut = LPWSTR;

// windows uses utf-16 :(
static WinStrConst from_trippin_to_win32_str(tr::String str)
{
	// conveniently microsoft knows this is torture and gives a function for this exact purpose
	int size = MultiByteToWideChar(CP_UTF8, 0, str.buf(), -1, nullptr, 0);
	// TODO idk if it can go negative but the headers don't have documentation, and the search online button uses bing
	// let's just say, that this table right here, is bill gates! *smashes table* YEAHHHHHHHHHHHHHH
	// https://www.youtube.com/watch?v=WGFLPbpdMS8
	TR_ASSERT_MSG(size != 0, "blame it on windows");

	WinStrMut new_str = static_cast<WinStrMut>(tr::scratchpad().alloc((size + 1) * sizeof(wchar_t)));
	int result = MultiByteToWideChar(CP_UTF8, 0, str.buf(), -1, new_str, size);
	TR_ASSERT_MSG(result != 0, "blame it on windows");
	return new_str;
}

static tr::String from_win32_to_trippin_str(WinStrConst str)
{
	int size = WideCharToMultiByte(CP_UTF8, 0, str, -1, nullptr, 0, nullptr, nullptr);
	TR_ASSERT_MSG(size != 0, "blame it on windows");

	tr::String new_str(tr::scratchpad(), size);
	int result = WideCharToMultiByte(CP_UTF8, 0, str, -1, new_str.buf(), size, nullptr, nullptr);
	TR_ASSERT_MSG(result != 0, "blame it on windows");
	return new_str;
}

tr::Result<tr::File&, tr::FileError> tr::File::open(tr::Arena& arena, tr::String path, FileMode mode)
{
	FileError::reset_errors();

	// get mode
	WinStrConst modefrfr = L"";
	switch (mode) {
		// on text mode windows does evil fuckery that we don't want
		// we want everything to be unix like
		// so we have to use binary mode
		case FileMode::READ_TEXT:         modefrfr = L"rb";  break;
		case FileMode::READ_BINARY:       modefrfr = L"rb";  break;
		case FileMode::WRITE_TEXT:        modefrfr = L"wb";  break;
		case FileMode::WRITE_BINARY:      modefrfr = L"wb";  break;
		case FileMode::READ_WRITE_TEXT:   modefrfr = L"rb+"; break;
		case FileMode::READ_WRITE_BINARY: modefrfr = L"rb+"; break;
		default:                          modefrfr = L"";    break;
	}

	File& file = arena.make<File>();
	// normal fopen gives an error on visual studio??
	errno_t ohno = _wfopen_s(reinterpret_cast<FILE**>(&file.fptr), from_trippin_to_win32_str(path), modefrfr);
	if (ohno != 0) return FileError::from_errno(path, "", FileOperation::OPEN_FILE);

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

tr::File::~File()
{
	FileError::reset_errors();
	this->close();
}

tr::Result<int64, const tr::Error&> tr::File::position()
{
	FileError::reset_errors();

	int64 pos = _ftelli64(static_cast<FILE*>(this->fptr));
	if (pos < 0) return FileError::from_errno(this->path, "", FileOperation::GET_FILE_POSITION);
	else return pos;
}

tr::Result<int64, const tr::Error&> tr::File::len()
{
	FileError::reset_errors();
	return this->length;
}

tr::Result<bool, const tr::Error&> tr::File::eof()
{
	FileError::reset_errors();

	return feof(static_cast<FILE*>(this->fptr)) != 0;
}

tr::Result<void, const tr::Error&> tr::File::seek(int64 bytes, tr::SeekFrom from)
{
	FileError::reset_errors();

	int whence = SEEK_CUR;
	switch (from) {
		case SeekFrom::START:   whence = SEEK_SET; break;
		case SeekFrom::CURRENT: whence = SEEK_CUR; break;
		case SeekFrom::END:     whence = SEEK_END; break;
	}

	int i = _fseeki64(static_cast<FILE*>(this->fptr), bytes, whence);
	if (i != 0) return FileError::from_errno(this->path, "", FileOperation::SEEK_FILE);
	else return {};
}

tr::Result<void, const tr::Error&> tr::File::rewind()
{
	FileError::reset_errors();

	::rewind(static_cast<FILE*>(this->fptr));
	if (errno != 0) return FileError::from_errno(this->path, "", FileOperation::REWIND_FILE);
	else return {};
}

tr::Result<int64, const tr::Error&> tr::File::read_bytes(void* out, int64 size, int64 items)
{
	FileError::reset_errors();
	TR_ASSERT_MSG(out != nullptr, "you dumbass it's supposed to go somewhere if you don't want to use it use File::seek() dumbass");
	TR_ASSERT_MSG(this->can_read(), "dumbass you can't read this file");

	usize bytes = fread(out, size, items, static_cast<FILE*>(this->fptr));
	if (errno != 0) return FileError::from_errno(this->path, "", FileOperation::READ_FILE);
	else return bytes;
}

tr::Result<void, const tr::Error&> tr::File::flush()
{
	FileError::reset_errors();

	int i = fflush(static_cast<FILE*>(this->fptr));
	if (i == EOF) return FileError::from_errno(this->path, "", FileOperation::FLUSH_FILE);
	else return {};
}

tr::Result<void, const tr::Error&> tr::File::write_bytes(Array<uint8> bytes)
{
	FileError::reset_errors();
	TR_ASSERT_MSG(this->can_write(), "dumbass you can't write to this file");

	fwrite(bytes.buf(), sizeof(uint8), bytes.len(), static_cast<FILE*>(this->fptr));
	if (errno != 0) return FileError::from_errno(this->path, "", FileOperation::WRITE_FILE);
	else return {};
}

bool tr::File::can_read()
{
	switch (this->mode) {
		case FileMode::READ_TEXT:          return true;
		case FileMode::READ_BINARY:        return true;
		case FileMode::WRITE_TEXT:         return false;
		case FileMode::WRITE_BINARY:       return false;
		case FileMode::READ_WRITE_TEXT:    return true;
		case FileMode::READ_WRITE_BINARY:  return true;
		default:                           return false;
	}
}

bool tr::File::can_write()
{
	switch (this->mode) {
		case FileMode::READ_TEXT:          return false;
		case FileMode::READ_BINARY:        return false;
		case FileMode::WRITE_TEXT:         return true;
		case FileMode::WRITE_BINARY:       return true;
		case FileMode::READ_WRITE_TEXT:    return true;
		case FileMode::READ_WRITE_BINARY:  return true;
		default:                           return false;
	}
}

tr::Result<void, tr::FileError> tr::remove_file(tr::String path)
{
	FileError::reset_errors();

	remove(path);
	if (errno != 0) return FileError::from_errno(path, "", FileOperation::REMOVE_FILE);
	else return {};
}

tr::Result<void, tr::FileError> tr::move_file(tr::String from, tr::String to)
{
	FileError::reset_errors();

	// libc rename() is different on windows and posix
	// on posix it replaces the destination if it already exists
	// on windows it fails in that case
	if (tr::file_exists(to)) return FileError(from, to, FileErrorType::FILE_EXISTS, FileOperation::MOVE_FILE);

	int i = rename(from, to);
	if (i == -1) return FileError::from_errno(from, to, FileOperation::MOVE_FILE);
	else return {};
}

bool tr::file_exists(tr::String path)
{
	DWORD attr = GetFileAttributesW(from_trippin_to_win32_str(path));
	return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

tr::Result<void, tr::FileError> tr::create_dir(tr::String path)
{
	FileError::reset_errors();

	// it's recursive :)
	Array<String> dirs = path.replace(tr::scratchpad(), '\\', '/').split(tr::scratchpad(), '/');

	for (auto [_, dir] : dirs) {
		if (tr::file_exists(dir)) continue;

		if (!CreateDirectoryW(from_trippin_to_win32_str(dir), nullptr)) {
			return FileError::from_win32(path, "", FileOperation::CREATE_DIR);
		}
	}

	return {};
}

tr::Result<void, tr::FileError> tr::remove_dir(tr::String path)
{
	FileError::reset_errors();

	if (!RemoveDirectoryW(from_trippin_to_win32_str(path))) {
		return FileError::from_win32(path, "", FileOperation::REMOVE_DIR);
	}
	return {};
}

tr::Result<tr::Array<tr::String>, tr::FileError> tr::list_dir(tr::Arena& arena, tr::String path,
	bool include_hidden)
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
		if (wcscmp(find_file_data.cFileName, L".") == 0) continue;
		if (wcscmp(find_file_data.cFileName, L"..") == 0) continue;

		if (!include_hidden) {
			if (find_file_data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) continue;
		}

		entries.add(from_win32_to_trippin_str(find_file_data.cFileName));
	}
	while (FindNextFileW(hfind, &find_file_data) != 0);

	FindClose(hfind);
	return entries;
}

tr::Result<bool, tr::FileError> tr::is_file(tr::String path)
{
	DWORD attributes = GetFileAttributesW(from_trippin_to_win32_str(path));

	if (attributes == INVALID_FILE_ATTRIBUTES) {
		return FileError::from_win32(path, "", FileOperation::IS_FILE);
	}

	return !(attributes & FILE_ATTRIBUTE_DIRECTORY);
}

void tr::__init_paths()
{
	// we're first getting it as utf-16 then converting it back to utf-8 just in case lmao
	WinStrMut exedir = static_cast<WinStrMut>(tr::core_arena.alloc(MAX_PATH * sizeof(wchar_t)));
	HMODULE hmodule = GetModuleHandle(nullptr);
	if (hmodule != nullptr) {
		DWORD len = GetModuleFileNameW(hmodule, exedir, MAX_PATH);
		if (len == 0) {
			tr::warn("couldn't get executable directory, using relative paths for app://");
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
	char buf[MAX_PATH] = {};
	// HOW IS char(*)[] INCOMPATIBLE WITH char**
	_dupenv_s(reinterpret_cast<char**>(&buf), nullptr, "APPDATA");
	tr::user_dir = String(buf).duplicate(tr::core_arena);
	#else
	char* appdata = getenv("APPDATA");
	tr::user_dir = String(appdata).duplicate(tr::core_arena);
	#endif
}

#else
/*
 * POSIX IMPLEMENTATION
 */

tr::Result<tr::File&, tr::FileError> tr::File::open(tr::Arena& arena, tr::String path, tr::FileMode mode)
{
	FileError::reset_errors();

	// get mode
	String modefrfr;
	switch (mode) {
		case FileMode::READ_TEXT:         modefrfr = "r";   break;
		case FileMode::READ_BINARY:       modefrfr = "rb";  break;
		case FileMode::WRITE_TEXT:        modefrfr = "w";   break;
		case FileMode::WRITE_BINARY:      modefrfr = "wb";  break;
		case FileMode::READ_WRITE_TEXT:   modefrfr = "r+";  break;
		case FileMode::READ_WRITE_BINARY: modefrfr = "rb+"; break;
		default:                          modefrfr = "";    break;
	}

	File& file = arena.make<File>();
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

tr::File::~File()
{
	FileError::reset_errors();
	this->close();
}

tr::Result<int64, const tr::Error&> tr::File::position()
{
	FileError::reset_errors();

	int64 pos = ftell(static_cast<FILE*>(this->fptr));
	if (pos < 0) return FileError::from_errno(this->path, "", FileOperation::GET_FILE_POSITION);
	else return pos;
}

tr::Result<int64, const tr::Error&> tr::File::len()
{
	FileError::reset_errors();
	return this->length;
}

tr::Result<bool, const tr::Error&> tr::File::eof()
{
	FileError::reset_errors();

	return feof(static_cast<FILE*>(this->fptr)) != 0;
}

tr::Result<void, const tr::Error&> tr::File::seek(int64 bytes, tr::SeekFrom from)
{
	FileError::reset_errors();

	int whence = SEEK_CUR;
	switch (from) {
		case SeekFrom::START:   whence = SEEK_SET; break;
		case SeekFrom::CURRENT: whence = SEEK_CUR; break;
		case SeekFrom::END:     whence = SEEK_END; break;
	}

	int i = fseek(static_cast<FILE*>(this->fptr), bytes, whence);
	if (i != 0) return FileError::from_errno(this->path, "", FileOperation::SEEK_FILE);
	else return {};
}

tr::Result<void, const tr::Error&> tr::File::rewind()
{
	FileError::reset_errors();

	::rewind(static_cast<FILE*>(this->fptr));
	if (errno != 0) return FileError::from_errno(this->path, "", FileOperation::REWIND_FILE);
	else return {};
}

tr::Result<int64, const tr::Error&> tr::File::read_bytes(void* out, int64 size, int64 items)
{
	FileError::reset_errors();
	// TODO TR_TRY_ASSERT
	TR_ASSERT_MSG(out != nullptr, "you dumbass it's supposed to go somewhere if you don't want to use it use File::seek() dumbass");
	TR_ASSERT_MSG(this->can_read(), "dumbass you can't read this file");

	// TODO 32-bit won't be happy about this
	usize bytes = fread(out, static_cast<usize>(size), static_cast<usize>(items),
		static_cast<FILE*>(this->fptr));

	if (errno != 0) return FileError::from_errno(this->path, "", FileOperation::READ_FILE);
	else return static_cast<int64>(bytes);
}

tr::Result<void, const tr::Error&> tr::File::flush()
{
	FileError::reset_errors();

	int i = fflush(static_cast<FILE*>(this->fptr));
	if (i == EOF) return FileError::from_errno(this->path, "", FileOperation::FLUSH_FILE);
	else return {};
}

tr::Result<void, const tr::Error&> tr::File::write_bytes(Array<uint8> bytes)
{
	FileError::reset_errors();
	TR_ASSERT_MSG(this->can_write(), "dumbass you can't write to this file");

	fwrite(bytes.buf(), sizeof(uint8), bytes.len(), static_cast<FILE*>(this->fptr));
	if (errno != 0) return FileError::from_errno(this->path, "", FileOperation::WRITE_FILE);
	else return {};
}

bool tr::File::can_read()
{
	switch (this->mode) {
		case FileMode::READ_TEXT:          return true;
		case FileMode::READ_BINARY:        return true;
		case FileMode::WRITE_TEXT:         return false;
		case FileMode::WRITE_BINARY:       return false;
		case FileMode::READ_WRITE_TEXT:    return true;
		case FileMode::READ_WRITE_BINARY:  return true;
		default:                           return false;
	}
}

bool tr::File::can_write()
{
	switch (this->mode) {
		case FileMode::READ_TEXT:          return false;
		case FileMode::READ_BINARY:        return false;
		case FileMode::WRITE_TEXT:         return true;
		case FileMode::WRITE_BINARY:       return true;
		case FileMode::READ_WRITE_TEXT:    return true;
		case FileMode::READ_WRITE_BINARY:  return true;
		default:                           return false;
	}
}

tr::Result<void, tr::FileError> tr::remove_file(tr::String path)
{
	FileError::reset_errors();

	remove(path);
	if (errno != 0) return FileError::from_errno(path, "", FileOperation::REMOVE_FILE);
	else return {};
}

tr::Result<void, tr::FileError> tr::move_file(tr::String from, tr::String to)
{
	FileError::reset_errors();

	// libc rename() is different on windows and posix
	// on posix it replaces the destination if it already exists
	// on windows it fails in that case
	if (tr::file_exists(to)) return FileError(from, to, FileErrorType::FILE_EXISTS, FileOperation::MOVE_FILE);

	int i = rename(from, to);
	if (i == -1) return FileError::from_errno(from, to, FileOperation::MOVE_FILE);
	else return {};
}

bool tr::file_exists(tr::String path)
{
	FileError::reset_errors();

	// we could just fopen(path, "r") then check if that's null, but then it would return false on permission
	// errors, even though it does in fact exist
	struct stat buffer;
	return stat(path, &buffer) == 0;
}

tr::Result<void, tr::FileError> tr::create_dir(tr::String path)
{
	FileError::reset_errors();

	// it's recursive :)
	Array<String> dirs = path.split(tr::scratchpad(), '/');

	for (auto [_, dir] : dirs) {
		if (tr::file_exists(dir)) continue;

		if (mkdir(dir, 0755) == -1) {
			return FileError::from_errno(path, "", FileOperation::CREATE_DIR);
		}
	}

	return {};
}

tr::Result<void, tr::FileError> tr::remove_dir(tr::String path)
{
	FileError::reset_errors();

	if (rmdir(path) != 0) {
		return FileError::from_errno(path, "", FileOperation::REMOVE_DIR);
	}
	return {};
}

tr::Result<tr::Array<tr::String>, tr::FileError> tr::list_dir(tr::Arena& arena, tr::String path,
	bool include_hidden)
{
	FileError::reset_errors();

	DIR* dir = opendir(path);
	if (dir == nullptr) {
		return FileError::from_errno(path, "", FileOperation::LIST_DIR);
	}

	Array<String> entries(arena);
	struct dirent* entry;

	while ((entry = readdir(dir)) != nullptr) {
		if (String(entry->d_name) == ".") continue;
		if (String(entry->d_name) == "..") continue;

		if (!include_hidden) {
			if (String(entry->d_name).starts_with(".")) continue;
		}

		entries.add(String(arena, entry->d_name, strlen(entry->d_name)));
	}

	return entries;
}

tr::Result<bool, tr::FileError> tr::is_file(tr::String path)
{
	FileError::reset_errors();

	struct stat statma;
	if (stat(path, &statma) != 0) {
		return FileError::from_errno(path, "", FileOperation::IS_FILE);
	}

	// TODO there's other types but they're similar to files so i'm counting all of them as files too
	if (S_ISDIR(statma.st_mode)) {
		return false;
	}
	else {
		return true;
	}
}

void tr::__init_paths()
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
		tr::exe_dir[usize(len)] = '\0';
		tr::exe_dir = tr::exe_dir.directory(tr::core_arena);
	}

	char* home = getenv("HOME");
	tr::appdata_dir = tr::fmt(tr::core_arena, "%s/.local/share", home);
}

#endif
