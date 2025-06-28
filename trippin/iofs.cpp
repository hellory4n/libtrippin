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

#include "collection.hpp"

#include "iofs.hpp"

// man. these are initialized in tr::init
namespace tr {
	Ref<File> std_in;
	Ref<File> std_out;
	Ref<File> std_err;
}

tr::Maybe<tr::String> tr::Reader::read_string(tr::Ref<tr::Arena> arena, usize length)
{
	String str(arena, length);
	uint64 read = this->read_bytes(str.buffer(), sizeof(char), length);
	if (read == length) return {};
	else return str;
}

tr::String tr::Reader::read_line(Ref<Arena> arena)
{
	Ref<List<char>> linema = new List<char>();

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
		linema->add(byte);
	}

	return String(arena, linema->buffer(), linema->length() + 1);
}

tr::Array<uint8> tr::Reader::read_all_bytes(tr::Ref<tr::Arena> arena)
{
	Array<uint8> man(arena, this->length());
	this->read_bytes(man.buffer(), sizeof(uint8), this->length());
	return man;
}

tr::String tr::Reader::read_all_text(tr::Ref<tr::Arena> arena)
{
	String str(arena, this->length());
	this->read_bytes(str.buffer(), sizeof(uint8), this->length());
	return str;
}

void tr::Writer::write_string(tr::String str, bool include_len)
{
	if (include_len) {
		this->write_struct(str.length());
	}

	Array<uint8> manfuckyou(reinterpret_cast<uint8*>(str.buffer()), str.length());
	this->write_bytes(manfuckyou);
}

tr::Maybe<tr::Ref<tr::File>> tr::File::open(tr::String path, FileMode mode)
{
	// get mode
	String modefrfr;
	switch (mode) {
		case FileMode::UNKNOWN:           modefrfr = "";    break;
		case FileMode::READ_TEXT:         modefrfr = "r";   break;
		case FileMode::READ_BINARY:       modefrfr = "rb";  break;
		case FileMode::WRITE_TEXT:        modefrfr = "w";   break;
		case FileMode::WRITE_BINARY:      modefrfr = "wb";  break;
		case FileMode::READ_WRITE_TEXT:   modefrfr = "r+";  break;
		case FileMode::READ_WRITE_BINARY: modefrfr = "rb+"; break;
	}

	Ref<File> file = new File();
	file->fptr = fopen(path, modefrfr);
	if (file->fptr == nullptr) return {};

	file->is_std = false;
	file->mode = mode;

	// get length :)))))))))
	fseek(file->fptr, 0, SEEK_END);
	file->len = ftell(file->fptr);
	::rewind(file->fptr);

	return file;
}

tr::File::~File()
{
	// is_std is so it doesn't close tr::std_out and company
	if (this->is_std || this->fptr != nullptr) return;
	fclose(this->fptr);
}

// TODO consider being less offensive?
// not offensive as in "libtrippin has gone woke :("

int64 tr::File::position()
{
	TR_ASSERT_MSG(this->fptr != nullptr, "uninitialized tr::File, initialize it you dumbass");
	return ftell(this->fptr);
}

int64 tr::File::length()
{
	TR_ASSERT_MSG(this->fptr != nullptr, "uninitialized tr::File, initialize it you dumbass");
	return this->len;
}

bool tr::File::eof()
{
	TR_ASSERT_MSG(this->fptr != nullptr, "uninitialized tr::File, initialize it you dumbass");
	return feof(this->fptr) != 0;
}

void tr::File::seek(uint64 bytes, tr::SeekFrom from)
{
	TR_ASSERT_MSG(this->fptr != nullptr, "uninitialized tr::File, initialize it you dumbass");

	int whence;
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

	fwrite(bytes.buffer(), sizeof(uint8), bytes.length(), this->fptr);
}

FILE* tr::File::cfile()
{
	return this->fptr;
}

bool tr::File::can_read()
{
	switch (this->mode) {
		case FileMode::UNKNOWN:           return false;
		case FileMode::READ_TEXT:         return true;
		case FileMode::READ_BINARY:       return true;
		case FileMode::WRITE_TEXT:        return false;
		case FileMode::WRITE_BINARY:      return false;
		case FileMode::READ_WRITE_TEXT:   return true;
		case FileMode::READ_WRITE_BINARY: return true;
	}
}

bool tr::File::can_write()
{
	switch (this->mode) {
		case FileMode::UNKNOWN:           return false;
		case FileMode::READ_TEXT:         return false;
		case FileMode::READ_BINARY:       return false;
		case FileMode::WRITE_TEXT:        return true;
		case FileMode::WRITE_BINARY:      return true;
		case FileMode::READ_WRITE_TEXT:   return true;
		case FileMode::READ_WRITE_BINARY: return true;
	}
}

bool tr::File::remove(tr::String path)
{
	return ::remove(path) == 0;
}

bool tr::File::rename(tr::String from, tr::String to)
{
	return ::rename(from, to) == 0;
}
