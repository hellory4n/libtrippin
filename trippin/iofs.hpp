/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/iofs.hpp
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

#ifndef _TRIPPIN_IOFS_H
#define _TRIPPIN_IOFS_H

#include "common.hpp"
#include "memory.hpp"
#include "string.hpp"
#include "error.hpp"

namespace tr {

enum class SeekFrom { START, CURRENT, END };

// Interface for reading streams of bytes
class Reader
{
public:
	// shut up
	virtual ~Reader() {}

	// Returns the current position of the cursor, if available
	virtual Result<int64, Error> position() = 0;

	// Returns the length of the stream in bytes, if available
	virtual Result<int64, Error> len() = 0;

	// If true, the stream ended.
	virtual Result<bool, Error> eof() = 0;

	// Moves the cursor without reading anything
	virtual Maybe<Error> seek(int64 bytes, SeekFrom from) = 0;

	// Goes back to the beginning of the stream, if available
	virtual Maybe<Error> rewind() = 0;

	// Reads any amount of bytes, and returns how many bytes were actually read.
	virtual Result<int64, Error> read_bytes(void* out, int64 size, int64 items) = 0;

	// Wrapper for `read_bytes`, returns null if it couldn't read the struct
	template<typename T>
	Maybe<T> read_struct()
	{
		T man;
		Result<int64> sir = this->read_bytes(&man, sizeof(T), 1);
		if (!sir.is_valid()) return {};

		if (sir.unwrap() == sizeof(T)) return man;
		else return {};
	}

	// Wrapper for `read_bytes`, returns an array of N items or null if it isn't able to read the stream.
	template<typename T>
	Maybe<Array<T>> read_array(Arena& arena, usize items)
	{
		T* man;
		Result<int64> sir = this->read_bytes(&man, sizeof(T), items);
		if (!sir.is_valid()) return {};

		if (sir.unwrap() == sizeof(T) * items && man != nullptr) return Array<T>(arena, man, items);
		else return {};
	}

	// Wrapper for `read_bytes`, returns a string or null if it isn't able to read the stream.
	Maybe<String> read_string(Arena& arena, usize length);

	// Reads a line of text :) Supports both Unix `\n` and Windows `\r\n`, no one is gonna be using classic
	// MacOS with this
	Maybe<String> read_line(Arena& arena);

	// Reads the entire stream as bytes
	Maybe<Array<uint8>> read_all_bytes(Arena& arena);

	// Reads the entire stream as text
	Maybe<String> read_all_text(Arena& arena);

	// TODO scanf or whatever the fuck
};

// Interface for writing to streams of bytes
class Writer
{
public:
	// shut up
	virtual ~Writer() {}

	// It flushes the stream :)
	virtual Maybe<Error> flush() = 0;

	// Writes bytes into the stream
	virtual Maybe<Error> write_bytes(Array<uint8> bytes) = 0;

	// Writes a struct into the stream
	template<typename T>
	Maybe<Error> write_struct(T data)
	{
		Array<uint8> manfuckyou(reinterpret_cast<uint8*>(&data), sizeof(T));
		return this->write_bytes(manfuckyou);
	}

	// Writes an array into the stream. If `include_len` is true, it'll include an uint64 with the length
	// (in items, not bytes) before the actual data.
	template<typename T>
	Maybe<Error> write_array(Array<T> array, bool include_len)
	{
		if (include_len) {
			Maybe<Error> mayhaps = this->write_struct(array.len());
			if (mayhaps.is_valid()) return mayhaps;
		}

		Array<uint8> manfuckyou(reinterpret_cast<uint8*>(array.buffer()), array.len());
		return this->write_bytes(manfuckyou);
	}

	// Writes a string into the stream. If `include_len` is true, it'll include an uint64 with the length
	// before the actual string.
	Maybe<Error> write_string(String str, bool include_len);

	// TODO printf or whatever the fuck
};

enum class FileMode : uint8 {
	UNKNOWN,
	// r, rb
	READ_TEXT, READ_BINARY,
	// w, wb
	WRITE_TEXT, WRITE_BINARY,
	// r+, rb+
	READ_WRITE_TEXT, READ_WRITE_BINARY,
};

// Files are definitely important. It's important to note that libtrippin ALWAYS uses forward slashes (`/`)
// for paths, as every platform supports them, even Windows (since 95/NT, both of which are pretty old).
// All backward slashes are automatically converted to forward slashes for consistency.
class File : public Reader, public Writer
{
	// so it can use it for errors
	String path = "";
	// i may eventually use windows' HANDLE
	// and including windows.h in a header is insane
	// i would rather die
	void* fptr = nullptr;
	// no need to calculate that more than once
	// probably
	// TODO what if there's a need to calculate that more than once
	int64 length = -1;
	// so it doesn't close stdout
	bool is_std = false;

	// just so it can check for read/write functions :)
	FileMode mode = FileMode::UNKNOWN;

	// it sets std_in/std_out/std_err lmao
	friend void tr::init();

public:
	File() {}
	~File();

	// Opens a fucking file from fucking somewhere. Returns null on error.
	static Result<File*, FileError> open(Arena& arena, String path, FileMode mode);

	// Closes the file :)
	void close();

	// Returns the current position of the cursor, if available
	Result<int64, Error> position() override;

	// Returns the length of the file in bytes, if available
	Result<int64, Error> len() override;

	// If true, the file ended. That's what "eof" means, End Of File
	Result<bool, Error> eof() override;

	// Moves the cursor without reading anything
	Maybe<Error> seek(int64 bytes, SeekFrom from) override;

	// Goes back to the beginning of the file.
	Maybe<Error> rewind() override;

	// Reads any amount of bytes, and returns how many bytes were actually read.
	Result<int64, Error> read_bytes(void* out, int64 size, int64 items) override;

	// It flushes the stream :)
	Maybe<Error> flush() override;

	// Writes bytes into the stream
	Maybe<Error> write_bytes(Array<uint8> bytes) override;

	// If true, the file can be read.
	bool can_read();

	// If true, the file can be written to.
	bool can_write();
};

// idk why not
// the _ is bcuz stdin/stdout/stderr are macros according to the standard
// it's safe in gcc/clang (bcuz it becomes the same name) but it's not ideal

// `stdin` but `tr::File`.
extern File std_in;
// `stdout` but `tr::File`.
extern File std_out;
// `stderr` but `tr::File`.
extern File std_err;

// Removes a file from a path
Maybe<Error> remove_file(String path);

// Moves or renames a file, returns true if it succeeds. Note this fails if the destination already exists
// (unlike posix's `rename()` which overwrites the destination)
Maybe<Error> move_file(String from, String to);

// Returns true if the file exists
bool file_exists(String path);

// Copies a file lmao.
Maybe<Error> copy_file(String src, String dst);

// Creates a directory. This is recursive, so `tr::create_dir("dir/otherdir")` will make both `dir`
// and `otherdir`.
Maybe<Error> create_dir(String path);

// Removes a directory. You can only remove empty directories, if you want to remove their contents you'll
// have to do that yourself.
Maybe<Error> remove_dir(String path);

// Lists all the files/directories
Result<Array<String>, Error> list_dir(Arena& arena, String path);

// If true, the path is a file. Else, it's a directory.
Result<bool, Error> is_file(String path);

}

#endif
