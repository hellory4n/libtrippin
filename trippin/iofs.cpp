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
#else
	#include <stdio.h>
	#include <sys/stat.h>
	#include <sys/types.h>
	#include <dirent.h>
	#include <unistd.h>
	#include <errno.h>
#endif

#include "log.hpp"

#include "iofs.hpp"

tr::Result<tr::String, tr::Error> tr::Reader::read_string(tr::Arena& arena, usize length)
{
	String str(arena, length);
	Result<int64, Error> read = this->read_bytes(str.buf(), sizeof(char), length);
	if (!read.is_valid()) return Result<String, Error>(read.unwrap_err());

	if (read.unwrap() != int64(length)) {
		return StringError(
			tr::sprintf(tr::scratchpad, "expected %li bytes, got %li (might be EOF)",
				length, read.unwrap()
			)
		);
	}
	else return str;
}

tr::Result<tr::String, tr::Error> tr::Reader::read_line(Arena& arena)
{
	Array<char> linema(tr::scratchpad, 0);

	while (true) {
		char byte = '\0';
		Result<int64, Error> result = this->read_bytes(&byte, sizeof(char), 1);
		if (!result.is_valid()) return Result<String, Error>(result.unwrap_err());
		int64 read = result.unwrap();

		// eof? idfk man
		if (read == 0) break;

		// i love unixcxfukc
		if (byte == '\n') break;

		// windows :(
		if (byte == '\r') {
			char next_byte = '\0';
			result = this->read_bytes(&next_byte, sizeof(char), 1);
			if (!result.is_valid()) return Result<String, Error>(result.unwrap_err());
			read = result.unwrap();

			// eof still counts
			if (read == 0 || next_byte == '\n') break;
		}

		// normal character
		linema.add(byte);
	}

	if (linema.len() == 0) return String("");
	else return String(arena, linema.buf(), linema.len() + 1);
}

tr::Result<tr::Array<uint8>, tr::Error> tr::Reader::read_all_bytes(tr::Arena& arena)
{
	Result<int64, Error> length = this->len();
	if (!length.is_valid()) return Result<Array<uint8>, Error>(length.unwrap_err());
	int64 lenfrfr = length.unwrap();

	Array<uint8> man(arena, lenfrfr);
	Result<int64, Error> die = this->read_bytes(man.buf(), sizeof(uint8), lenfrfr);
	if (die.is_valid()) return man;
	else return Result<Array<uint8>, Error>(die.unwrap_err());
}

tr::Result<tr::String, tr::Error> tr::Reader::read_all_text(tr::Arena& arena)
{
	Result<int64, Error> length = this->len();
	if (!length.is_valid()) return Result<String, Error>(length.unwrap_err());
	int64 lenfrfr = length.unwrap();

	String man(arena, lenfrfr);
	Result<int64, Error> die = this->read_bytes(man.buf(), sizeof(char), lenfrfr);
	if (die.is_valid()) return man;
	else return Result<String, Error>(die.unwrap_err());
}

tr::Result<void, tr::Error> tr::Writer::write_string(tr::String str, bool include_len)
{
	if (include_len) {
		Result<void, Error> help = this->write_struct(str.len());
		if (!help.is_valid()) return help;
	}

	Array<uint8> manfuckyou(reinterpret_cast<uint8*>(str.buf()), str.len());
	return this->write_bytes(manfuckyou);
}

#ifdef _WIN32
/*
 * WINDOWS IMPLEMENTATION
 */
// TODO use only windows APIs (massive pain in the ass)
#else
/*
 * POSIX IMPLEMENTATION
 */

tr::Result<tr::File*, tr::FileError> tr::File::open(tr::Arena& arena, tr::String path, FileMode mode)
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
	if (file.fptr == nullptr) return FileError::from_errno(path, "", FileOperation::OPEN_FILE);

	file.is_std = false;
	file.mode = mode;
	file.path = path;

	// get length :)))))))))
	fseek(reinterpret_cast<FILE*>(file.fptr), 0, SEEK_END);
	file.length = ftell(reinterpret_cast<FILE*>(file.fptr));
	::rewind(reinterpret_cast<FILE*>(file.fptr));

	return &file;
}

void tr::File::close()
{
	FileError::reset_errors();

	// is_std exists so it doesn't close tr::std_out and company
	if (!this->is_std && this->fptr != nullptr) {
		fclose(reinterpret_cast<FILE*>(this->fptr));
	}
	this->fptr = nullptr;
}

tr::File::~File()
{
	FileError::reset_errors();
	this->close();
}

tr::Result<int64, tr::Error> tr::File::position()
{
	FileError::reset_errors();

	// TODO windows is stupid so ftell doesn't work with files that are more than 2 gb
	int64 pos = ftell(reinterpret_cast<FILE*>(this->fptr));
	if (pos < 0) return FileError::from_errno(this->path, "", FileOperation::GET_FILE_POSITION);
	else return pos;
}

tr::Result<int64, tr::Error> tr::File::len()
{
	FileError::reset_errors();
	return this->length;
}

tr::Result<bool, tr::Error> tr::File::eof()
{
	FileError::reset_errors();

	return feof(reinterpret_cast<FILE*>(this->fptr)) != 0;
}

tr::Result<void, tr::Error> tr::File::seek(int64 bytes, tr::SeekFrom from)
{
	FileError::reset_errors();

	int whence = SEEK_CUR;
	switch (from) {
		case SeekFrom::START:   whence = SEEK_SET; break;
		case SeekFrom::CURRENT: whence = SEEK_CUR; break;
		case SeekFrom::END:     whence = SEEK_END; break;
	}

	int i = fseek(reinterpret_cast<FILE*>(this->fptr), bytes, whence);
	if (i != 0) return FileError::from_errno(this->path, "", FileOperation::SEEK_FILE);
	else return {};
}

tr::Result<void, tr::Error> tr::File::rewind()
{
	FileError::reset_errors();

	::rewind(reinterpret_cast<FILE*>(this->fptr));
	if (errno != 0) return FileError::from_errno(this->path, "", FileOperation::REWIND_FILE);
	else return {};
}

tr::Result<int64, tr::Error> tr::File::read_bytes(void* out, int64 size, int64 items)
{
	FileError::reset_errors();
	TR_ASSERT_MSG(out != nullptr, "you dumbass it's supposed to go somewhere if you don't want to use it use File::seek() dumbass");
	TR_ASSERT_MSG(this->can_read(), "dumbass you can't read this file");

	usize bytes = fread(out, size, items, reinterpret_cast<FILE*>(this->fptr));
	if (errno != 0) return FileError::from_errno(this->path, "", FileOperation::READ_FILE);
	else return bytes;
}

tr::Result<void, tr::Error> tr::File::flush()
{
	FileError::reset_errors();

	int i = fflush(reinterpret_cast<FILE*>(this->fptr));
	if (i == EOF) return FileError::from_errno(this->path, "", FileOperation::FLUSH_FILE);
	else return {};
}

tr::Result<void, tr::Error> tr::File::write_bytes(Array<uint8> bytes)
{
	FileError::reset_errors();
	TR_ASSERT_MSG(this->can_write(), "dumbass you can't write to this file");

	fwrite(bytes.buf(), sizeof(uint8), bytes.len(), reinterpret_cast<FILE*>(this->fptr));
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

	rename(from, to);
	if (errno != 0) return FileError::from_errno(from, to, FileOperation::MOVE_FILE);
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

tr::Result<void, tr::FileError> tr::create_dir(tr::String)
{
	tr::panic("i didn't finish this function i'm busy uh getting milk");
	// TODO use String.split dumbass
	// TODO may i ask you what the fuck is this

	// // it's supposed to be recursive :D
	// Arena& tmp = new Arena(tr::kb_to_bytes(4));
	// Ref<List<String>> dirs = new List<String>();

	// usize idx = 0;
	// for (ArrayItem<char> c : path) {
	// 	if ((c.val != '/' && c.val != '\\') || c.i < path.len()) continue;

	// 	dirs->add(path.substr(tmp, idx, c.i));
	// 	idx = c.i + 1;
	// }
	// tr::panic("fuck...");
	// if (dirs->length() == 0) return false;

	// #ifdef _WIN32
	// #else
	// for (ArrayItem<String> dir : *dirs) {
	// 	if (mkdir(dir.val, 0755) != 0 && errno != EEXIST) {
	// 		return false;
	// 	}
	// }
	// tr::panic("hehe");
	// return true;
	// #endif
}

#endif
