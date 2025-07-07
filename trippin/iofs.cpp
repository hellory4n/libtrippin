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
	// #include <errno.h>
#endif

#include "log.hpp"

#include "iofs.hpp"

tr::Maybe<tr::String> tr::Reader::read_string(tr::Arena& arena, usize length)
{
	String str(arena, length);
	uint64 read = this->read_bytes(str.buf(), sizeof(char), length);
	if (read != length) return {};
	else return str;
}

tr::String tr::Reader::read_line(Arena& arena)
{
	Array<char> linema(tr::scratchpad, 0);

	while (true) {
		char byte = '\0';
		uint64 read = this->read_bytes(&byte, sizeof(char), 1);

		// eof? idfk man
		if (read == 0) break;

		// i love unixcxfukc
		if (byte == '\n') break;

		// windows :(
		if (byte == '\r') {
			char next_byte = '\0';
			read = this->read_bytes(&next_byte, sizeof(char), 1);

			// eof still counts
			if (read == 0 || next_byte == '\n') break;
		}

		// normal character
		linema.add(byte);
	}

	return String(arena, linema.buf(), linema.len() + 1);
}

tr::Array<uint8> tr::Reader::read_all_bytes(tr::Arena& arena)
{
	Array<uint8> man(arena, this->len());
	this->read_bytes(man.buf(), sizeof(uint8), this->len());
	return man;
}

tr::String tr::Reader::read_all_text(tr::Arena& arena)
{
	String str(arena, this->len());
	this->read_bytes(str.buf(), sizeof(uint8), this->len());
	return str;
}

void tr::Writer::write_string(tr::String str, bool include_len)
{
	if (include_len) {
		this->write_struct(str.len());
	}

	Array<uint8> manfuckyou(reinterpret_cast<uint8*>(str.buf()), str.len());
	this->write_bytes(manfuckyou);
}

tr::MaybePtr<tr::File> tr::File::open(tr::Arena& arena, tr::String path, FileMode mode)
{
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
	if (file.fptr == nullptr) return {};

	file.is_std = false;
	file.mode = mode;

	// get length :)))))))))
	fseek(reinterpret_cast<FILE*>(file.fptr), 0, SEEK_END);
	file.length = ftell(reinterpret_cast<FILE*>(file.fptr));
	::rewind(reinterpret_cast<FILE*>(file.fptr));

	return file;
}

void tr::File::close()
{
	// is_std exists so it doesn't close tr::std_out and company
	if (!this->is_std && this->fptr != nullptr) {
		fclose(reinterpret_cast<FILE*>(this->fptr));
		this->fptr = nullptr;
	}
}

tr::File::~File()
{
	this->close();
}

// TODO consider being less offensive?
// not offensive as in "libtrippin has gone woke :("

int64 tr::File::position()
{
	TR_ASSERT_MSG(this->fptr != nullptr, "uninitialized tr::File, initialize it you dumbass");
	return ftell(reinterpret_cast<FILE*>(this->fptr));
}

int64 tr::File::len()
{
	TR_ASSERT_MSG(this->fptr != nullptr, "uninitialized tr::File, initialize it you dumbass");
	return this->length;
}

bool tr::File::eof()
{
	TR_ASSERT_MSG(this->fptr != nullptr, "uninitialized tr::File, initialize it you dumbass");
	return feof(this->fptr) != 0;
}

void tr::File::seek(uint64 bytes, tr::SeekFrom from)
{
	TR_ASSERT_MSG(this->fptr != nullptr, "uninitialized tr::File, initialize it you dumbass");

	int whence = SEEK_CUR;
	switch (from) {
		case SeekFrom::START:   whence = SEEK_SET; break;
		case SeekFrom::CURRENT: whence = SEEK_CUR; break;
		case SeekFrom::END:     whence = SEEK_END; break;
	}

	fseek(this->fptr, bytes, whence);
}

bool tr::File::rewind()
{
	TR_ASSERT_MSG(this->fptr != nullptr, "uninitialized tr::File, initialize it you dumbass");
	::rewind(this->fptr);
	return true;
}

uint64 tr::File::read_bytes(void* out, uint64 size, uint64 items)
{
	TR_ASSERT_MSG(this->fptr != nullptr, "uninitialized tr::File, initialize it you dumbass");
	TR_ASSERT_MSG(out != nullptr, "you dumbass it's supposed to go somewhere if you don't want to use it use File::seek() dumbass");
	TR_ASSERT_MSG(this->can_read(), "dumbass you can't read this file");

	// TODO is this necessary?
	TR_ASSERT_MSG(size > 0, "dumbass why would you read 0 bytes");
	TR_ASSERT_MSG(items > 0, "dumbass why would you read 0 bytes");

	return fread(out, size, items, this->fptr);
}

void tr::File::flush()
{
	TR_ASSERT_MSG(this->fptr != nullptr, "uninitialized tr::File, initialize it you dumbass");
	fflush(this->fptr);
}

void tr::File::write_bytes(Array<uint8> bytes)
{
	TR_ASSERT_MSG(this->fptr != nullptr, "uninitialized tr::File, initialize it you dumbass");
	TR_ASSERT_MSG(this->can_write(), "dumbass you can't write to this file");

	fwrite(bytes.buf(), sizeof(uint8), bytes.len(), this->fptr);
}

FILE* tr::File::cfile()
{
	return this->fptr;
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

bool tr::remove_file(tr::String path)
{
	return remove(path) == 0;
}

bool tr::rename_file(tr::String from, tr::String to)
{
	// libc rename() is different on windows and posix
	// on posix it replaces the destination if it already exists
	// on windows it fails in that case

	if (tr::file_exists(to)) return false;

	#ifdef _WIN32
	return MoveFileEx(from, to, MOVEFILE_REPLACE_EXISTING) != 0;
	#else
	return rename(from, to) == 0;
	#endif
}

bool tr::file_exists(tr::String path)
{
	// we could just fopen(path, "r") then check if that's null, but then it would return false on permission
	// errors, even though it does in fact exist
	#ifdef _WIN32
	DWORD attrib = GetFileAttributesA(path);
	return attrib != INVALID_FILE_ATTRIBUTES && !(attrib & FILE_ATTRIBUTE_DIRECTORY);
	#else
	struct stat buffer;
    return stat(path, &buffer) == 0;
	#endif
}

bool tr::create_dir(tr::String)
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
