/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/iofs.h
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

#include "trippin/common.h"
#include "trippin/error.h"
#include "trippin/memory.h"
#include "trippin/string.h"

namespace tr {

enum class SeekFrom
{
	START,
	CURRENT,
	END
};

// Interface for reading streams of bytes
class Reader
{
public:
	// shut up
	virtual ~Reader() {}

	// Returns the current position of the cursor, if available
	virtual Result<int64, const Error&> position() = 0;

	// Returns the length of the stream in bytes, if available
	virtual Result<int64, const Error&> len() = 0;

	// If true, the stream ended.
	virtual Result<bool, const Error&> eof() = 0;

	// Moves the cursor without reading anything
	virtual Result<void, const Error&> seek(int64 bytes, SeekFrom from) = 0;

	// Goes back to the beginning of the stream, if available
	virtual Result<void, const Error&> rewind() = 0;

	// Reads any amount of bytes, and returns how many bytes were actually read.
	virtual Result<int64, const Error&> read_bytes(void* out, int64 size, int64 items) = 0;

	// Wrapper for `read_bytes`, returns null if it couldn't read the struct
	template <typename T>
	Result<T, const Error&> read_struct()
	{
		T man = nullptr;
		TR_TRY_ASSIGN(int64 bytes_read, this->read_bytes(&man, sizeof(T), 1));

		if (bytes_read == sizeof(T) && man != nullptr) {
			return man;
		}
		return tr::scratchpad().make<StringError>(
			"expected %li bytes, got %li (might be EOF)", sizeof(T), bytes_read);
	}

	// Wrapper for `read_bytes`, returns an array of N items or null if it isn't able to read
	// the stream.
	template <typename T>
	Result<Array<T>, const Error&> read_array(Arena& arena, int64 items)
	{
		T* man = nullptr;
		TR_TRY_ASSIGN(int64 bytes_read, this->read_bytes(&man, sizeof(T), items));

		if (bytes_read == sizeof(T) * items && man != nullptr) {
			return Array<T>(arena, man, items);
		}
		return tr::scratchpad().make<StringError>(
			"expected %li bytes, got %li (might be EOF)", sizeof(T) * items,
			bytes_read);
	}

	// Wrapper for `read_bytes`, returns a string or null if it isn't able to read the stream.
	Result<String, const Error&> read_string(Arena& arena, int64 length);

	// Reads a line of text :) Supports both Unix `\n` and Windows `\r\n`, no one is gonna be
	// using classic MacOS with this
	Result<String, const Error&> read_line(Arena& arena);

	// Reads the entire stream as bytes
	Result<Array<uint8>, const Error&> read_all_bytes(Arena& arena);

	// Reads the entire stream as text
	Result<String, const Error&> read_all_text(Arena& arena);

	// TODO scanf or whatever the fuck
	// or maybe not
};

// Interface for writing to streams of bytes
class Writer
{
public:
	// shut up
	virtual ~Writer() {}

	// It flushes the stream :)
	virtual Result<void, const Error&> flush() = 0;

	// Writes bytes into the stream
	virtual Result<void, const Error&> write_bytes(Array<uint8> bytes) = 0;

	// Writes a struct into the stream
	template <typename T>
	Result<void, const Error&> write_struct(T data)
	{
		Array<uint8> manfuckyou(reinterpret_cast<uint8*>(&data), sizeof(T));
		return this->write_bytes(manfuckyou);
	}

	// Writes an array into the stream. Note this doesn't include the length or a null
	// terminator, it just writes pure data into the stream.
	template <typename T>
	Result<void, const Error&> write_array(Array<T> array)
	{
		Array<uint8> manfuckyou(reinterpret_cast<uint8*>(array.buffer()), array.len());
		return this->write_bytes(manfuckyou);
	}

	// Writes a string into the stream. Note this doesn't include the length or a null
	// terminator, it just writes pure data into the stream.
	Result<void, const Error&> write_string(String str);

	// Writes a formatted string into the stream. So pretty much just fprintf.
	[[gnu::format(printf, 2, 3)]]
	// `this` is the first argument i guess
	Result<void, const Error&> printf(const char* fmt, ...);

	// Similar to `Writer.printf()`, but it adds a newline at the end.
	[[gnu::format(printf, 2, 3)]]
	// `this` is the first argument i guess
	Result<void, const Error&> println(const char* fmt, ...);

	// Writes an empty line. Mind-boggling.
	Result<void, const Error&> println()
	{
		return this->write_string("\n");
	}
};

enum class FileMode : uint8
{
	UNKNOWN,
	// r, rb
	READ_TEXT,
	READ_BINARY,
	// w, wb
	WRITE_TEXT,
	WRITE_BINARY,
	// r+, rb+
	READ_WRITE_TEXT,
	READ_WRITE_BINARY,
};

// Files are definitely important. It's important to note that libtrippin ALWAYS uses forward
// slashes (`/`) for paths, as every platform supports them, even Windows (since 95/NT, both of
// which are pretty old).
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
	friend void init();
	// it checks for is_std :)
	friend void _log(const char* color, const char* prefix, bool panic, const char* fmt,
			 va_list arg);

public:
	File() : path("") {}
	~File();

	// Opens a fucking file from fucking somewhere. Returns null on error.
	static Result<File&, FileError> open(Arena& arena, String path, FileMode mode);

	// Closes the file :)
	void close();

	// Returns the current position of the cursor, if available
	Result<int64, const Error&> position() override;

	// Returns the length of the file in bytes, if available
	Result<int64, const Error&> len() override;

	// If true, the file ended. That's what "eof" means, End Of File
	Result<bool, const Error&> eof() override;

	// Moves the cursor without reading anything
	Result<void, const Error&> seek(int64 bytes, SeekFrom from) override;

	// Goes back to the beginning of the file.
	Result<void, const Error&> rewind() override;

	// Reads any amount of bytes, and returns how many bytes were actually read.
	Result<int64, const Error&> read_bytes(void* out, int64 size, int64 items) override;

	// It flushes the stream :)
	Result<void, const Error&> flush() override;

	// Writes bytes into the stream
	Result<void, const Error&> write_bytes(Array<uint8> bytes) override;

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
Result<void, FileError> remove_file(String path);

// Moves or renames a file, returns true if it succeeds. Note this fails if the destination already
// exists (unlike posix's `rename()` which overwrites the destination)
Result<void, FileError> move_file(String from, String to);

// Returns true if the file exists
bool file_exists(String path);

// Creates a directory. This is recursive, so `tr::create_dir("dir/otherdir")` will make both `dir`
// and `otherdir`.
Result<void, FileError> create_dir(String path);

// Removes a directory. You can only remove empty directories, if you want to remove their contents
// you'll have to do that yourself.
Result<void, FileError> remove_dir(String path);

// Lists all the files/directories in a directory. The returned array has the paths relative to the
// directory path. The array does NOT include `.` and `..`. If `include_hidden` is false, it'll skip
// hidden files and directories. On POSIX that's anything that starts with a dot. Windows doesn't
// follow that convention and instead lets any file/directory be hidden.
Result<Array<String>, FileError> list_dir(Arena& arena, String path, bool include_hidden = true);

// If true, the path is a file. Else, it's a directory.
Result<bool, FileError> is_file(String path);

// Fancy path utility thing. The `app://` prefix is relative to the exectuable's directory, while
// `user://` refers to the directory intended for saving user crap (e.g. `%APPDATA%` on windows).
// You should configure this first with `tr::set_paths`
String path(Arena& arena, String path);

// Sets the paths used by `tr::path`. For example `tr::set_paths("assets", "handsome_app")`, or even
// `tr::set_paths("res", "BallsEnterprises/SpheresPro")`
void set_paths(String appdir, String userdir);

// internal :)
void _init_paths();

} // namespace tr

#endif
