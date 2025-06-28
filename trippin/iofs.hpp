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

namespace tr {

enum class SeekFrom { START, CURRENT, END };

// Interface for reading streams of bytes
class Reader : public RefCounted
{
	// Returns the current position of the cursor, or -1 if unknown/unsupported
	virtual int64 position() = 0;

	// Returns the length of the stream in bytes, or -1 if unknown/unsupported
	virtual int64 length() = 0;

	// If true, the stream ended.
	virtual bool eof() = 0;

	// Moves the cursor without reading anything
	virtual void seek(uint64 bytes) = 0;

	// Goes back to the beginning of the file, and returns true if that's supported.
	virtual bool rewind() = 0;

	// Reads any amount of bytes, and returns how many bytes were actually read.
	virtual uint64 read_bytes(void* out, uint64 size, uint64 items) = 0;

	// Wrapper for `read_bytes`, returns null if it couldn't read the struct
	template<typename T> Maybe<T> read_struct()
	{
		T man;
		uint64 bytes = this->read_bytes(&man, sizeof(T), 1);
		if (bytes == sizeof(T)) return man;
		else return {};
	}

	// Wrapper for `read_bytes`, returns an array of N items or null if it isn't able to read the stream.
	template<typename T> Maybe<Array<T>> read_array(Ref<Arena> arena, usize items)
	{
		T* man;
		uint64 bytes = this->read_bytes(&man, sizeof(T), items);
		if (bytes == sizeof(T) * items && man != nullptr) return Array<T>(arena, man, items);
		else return {};
	}

	// Reads the entire stream as bytes
	Array<uint8> read_all_bytes(Ref<Arena> arena);

	// Reads the entire stream as text
	String read_all_text(Ref<Arena> arena);

};

// Interface for writing to streams of bytes
class Writer : public RefCounted
{
	// Writes bytes into the stream
	virtual void write_bytes(Array<uint8> bytes) = 0;

	// Writes a struct into the stream
	template<typename T> void write_struct(T data)
	{
		Array<uint8> manfuckyou(reinterpret_cast<uint8*>(&data), sizeof(T));
		this->write_bytes(manfuckyou);
	}

	// Writes an array into the stream. If `include_len` is true, it'll include an uint64 with the length
	// (in items, not bytes) before the actual data.
	template<typename T> void write_array(Array<T> array, bool include_len)
	{
		if (include_len) {
			this->write_struct(array.length());
		}

		Array<uint8> manfuckyou(reinterpret_cast<uint8*>(array.buffer()), array.length());
		this->write_bytes(manfuckyou);
	}

	// Writes a string into the stream. If `include_len` is true, it'll include an uint64 with the length
	// before the actual string.
	void write_string(String str, bool include_len)
	{
		if (include_len) {
			this->write_struct(str.length());
		}

		Array<uint8> manfuckyou(reinterpret_cast<uint8*>(str.buffer()), str.length());
		this->write_bytes(manfuckyou);
	}

	// It flushes the stream :)
	virtual void flush() = 0;
};

}

#endif
