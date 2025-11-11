/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/error.h
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

#ifndef _TRIPPIN_ERROR_H
#define _TRIPPIN_ERROR_H

#include <type_traits>

#include "trippin/common.h"
#include "trippin/memory.h"
#include "trippin/string.h"

namespace tr {

// Rust devs cry at such beautiful type safety
union ErrorArg {
	uint8 u8 = 0;
	uint16 u16;
	uint32 u32;
	uint64 u64;
	int8 i8;
	int16 i16;
	int32 i32;
	int64 i64;
	float32 f32;
	float64 f64;
	String str;
	void* ptr;
	// Note `const char*` will be converted to `tr::String` so that string literals can still be
	// treated as libtrippin strings. You can still read it as a pointer due to `tr::String`'s
	// memory layout, but I wouldn't recommend.
	const void* const_ptr;
};

// Error metadata, which is useful for errors that need data. You get 16 slots which can be a
// arithmetic type, string, or void pointer. I can't imagine errors needing more than that.
using ErrorArgs = List<16, ErrorArg>;

// As the name implies, an error type. This is a number (arbitrary, usually from a string hash) that
// maps to a function that can get a string error message.
struct ErrorType
{
	uint64 id;

	constexpr ErrorType()
		: id(0)
	{
	}

	constexpr explicit ErrorType(uint64 x)
		: id(x)
	{
	}

	constexpr operator uint64() const
	{
		return id;
	}
};

// yea
constexpr ErrorType ERROR_OK = ErrorType{0};

// Does exactly what the name says. Manually choosing number values is gonna eventually cause
// collisions, so instead we use a hashing algorithm where that won't happen until we have a
// gazillion different error types (we don't)
consteval ErrorType errtype_from_string(const char* s)
{
	usize len = tr::strlib::constexpr_strlen(s);

	// yes im implementing the same hashing algorithm twice i dont care
	constexpr uint64 FNV_OFFSET_BASIS = 0xcbf29ce484222325;
	constexpr uint64 FNV_PRIME = 0x100000001b3;
	uint64 hash = FNV_OFFSET_BASIS;

	for (usize i = 0; i < len; i++) {
		hash ^= static_cast<uint8>(s[i]);
		hash *= FNV_PRIME;
	}

	return ErrorType{hash};
}

// Error type + args
struct Error
{
	ErrorArgs args;
	ErrorType type;
};

// Registers an error type. How amazing. Returns false if there's already an error with that ID,
// returns true otherwise.
bool register_error_type(ErrorType id, String (*msg_func)(ErrorArgs args));

// Slightly fancy macro to register error types right after the error message is declared, which
// looks nicer than 5000 `tr::register_error_type` calls in the main function.
#define TR_REGISTER_ERROR_TYPE(Id, MsgFunc)                                    \
	[[maybe_unused]]                                                       \
	static bool _tr_register_##Id = ::tr::register_error_type(Id, MsgFunc)

// Looks up the error type, calls its function, and returns the resulting error message. Panics if
// the ID is invalid (because it should never happen)
String error_message(ErrorType id, ErrorArgs meta);

// This is just for getting the error message lmao.
enum class FileOperation : int32
{
	UNKNOWN,
	OPEN_FILE,
	CLOSE_FILE,
	GET_FILE_POSITION,
	GET_FILE_LENGTH,
	IS_EOF,
	SEEK_FILE,
	REWIND_FILE,
	READ_FILE,
	FLUSH_FILE,
	WRITE_FILE,
	REMOVE_FILE,
	MOVE_FILE,
	CREATE_DIR,
	REMOVE_DIR,
	LIST_DIR,
	IS_FILE,
};

// some fucking bullshit

String errmsg_file_not_found(ErrorArgs meta);
// args: FileOperation as int32, string path A, string path B
constexpr ErrorType ERROR_FILE_NOT_FOUND = tr::errtype_from_string("tr::FILE_NOT_FOUND");
TR_REGISTER_ERROR_TYPE(ERROR_FILE_NOT_FOUND, errmsg_file_not_found);

String errmsg_access_denied(ErrorArgs meta);
// args: FileOperation as int32, string path A, string path B
constexpr ErrorType ERROR_ACCESS_DENIED = tr::errtype_from_string("tr::ACCESS_DENIED");
TR_REGISTER_ERROR_TYPE(ERROR_ACCESS_DENIED, errmsg_access_denied);

String errmsg_device_or_resource_busy(ErrorArgs meta);
// args: FileOperation as int32, string path A, string path B
constexpr ErrorType ERROR_DEVICE_OR_RESOURCE_BUSY =
	tr::errtype_from_string("tr::DEVICE_OR_RESOURCE_BUSY");
TR_REGISTER_ERROR_TYPE(ERROR_DEVICE_OR_RESOURCE_BUSY, errmsg_device_or_resource_busy);

String errmsg_no_space_left(ErrorArgs meta);
// args: FileOperation as int32, string path A, string path B
constexpr ErrorType ERROR_NO_SPACE_LEFT = tr::errtype_from_string("tr::NO_SPACE_LEFT");
TR_REGISTER_ERROR_TYPE(ERROR_NO_SPACE_LEFT, errmsg_no_space_left);

String errmsg_file_exists(ErrorArgs meta);
// args: FileOperation as int32, string path A, string path B
constexpr ErrorType ERROR_FILE_EXISTS = tr::errtype_from_string("tr::FILE_EXISTS");
TR_REGISTER_ERROR_TYPE(ERROR_FILE_EXISTS, errmsg_access_denied);

String errmsg_bad_handle(ErrorArgs meta);
// args: FileOperation as int32, string path A, string path B
constexpr ErrorType ERROR_BAD_HANDLE = tr::errtype_from_string("tr::BAD_HANDLE");
TR_REGISTER_ERROR_TYPE(ERROR_BAD_HANDLE, errmsg_access_denied);

String errmsg_hardware_error_or_unknown(ErrorArgs meta);
// args: FileOperation as int32, string path A, string path B
constexpr ErrorType ERROR_HARDWARE_ERROR_OR_UNKNOWN =
	tr::errtype_from_string("tr::HARDWARE_ERROR_OR_UNKNOWN");
TR_REGISTER_ERROR_TYPE(ERROR_HARDWARE_ERROR_OR_UNKNOWN, errmsg_hardware_error_or_unknown);

String errmsg_is_directory(ErrorArgs meta);
// args: FileOperation as int32, string path A, string path B
constexpr ErrorType ERROR_IS_DIRECTORY = tr::errtype_from_string("tr::IS_DIRECTORY");
TR_REGISTER_ERROR_TYPE(ERROR_IS_DIRECTORY, errmsg_is_directory);

String errmsg_is_not_directory(ErrorArgs meta);
// args: FileOperation as int32, string path A, string path B
constexpr ErrorType ERROR_IS_NOT_DIRECTORY = tr::errtype_from_string("tr::IS_NOT_DIRECTORY");
TR_REGISTER_ERROR_TYPE(ERROR_IS_NOT_DIRECTORY, errmsg_is_not_directory);

String errmsg_too_many_open_files(ErrorArgs meta);
// args: FileOperation as int32, string path A, string path B
constexpr ErrorType ERROR_TOO_MANY_OPEN_FILES = tr::errtype_from_string("tr::TOO_MANY_OPEN_FILES");
TR_REGISTER_ERROR_TYPE(ERROR_TOO_MANY_OPEN_FILES, errmsg_too_many_open_files);

String errmsg_broken_pipe(ErrorArgs meta);
// args: FileOperation as int32, string path A, string path B
constexpr ErrorType ERROR_BROKEN_PIPE = tr::errtype_from_string("tr::BROKEN_PIPE");
TR_REGISTER_ERROR_TYPE(ERROR_BROKEN_PIPE, errmsg_broken_pipe);

String errmsg_filename_too_long(ErrorArgs meta);
// args: FileOperation as int32, string path A, string path B
constexpr ErrorType ERROR_FILENAME_TOO_LONG = tr::errtype_from_string("tr::FILENAME_TOO_LONG");
TR_REGISTER_ERROR_TYPE(ERROR_FILENAME_TOO_LONG, errmsg_filename_too_long);

String errmsg_invalid_argument(ErrorArgs meta);
// args: FileOperation as int32, string path A, string path B
constexpr ErrorType ERROR_INVALID_ARGUMENT = tr::errtype_from_string("tr::INVALID_ARGUMENT");
TR_REGISTER_ERROR_TYPE(ERROR_INVALID_ARGUMENT, errmsg_invalid_argument);

String errmsg_read_only_filesystem(ErrorArgs meta);
// args: FileOperation as int32, string path A, string path B
constexpr ErrorType ERROR_READ_ONLY_FILESYSTEM =
	tr::errtype_from_string("tr::READ_ONLY_FILESYSTEM");
TR_REGISTER_ERROR_TYPE(ERROR_READ_ONLY_FILESYSTEM, errmsg_read_only_filesystem);

String errmsg_illegal_seek(ErrorArgs meta);
// args: FileOperation as int32, string path A, string path B
constexpr ErrorType ERROR_ILLEGAL_SEEK = tr::errtype_from_string("tr::ILLEGAL_SEEK");
TR_REGISTER_ERROR_TYPE(ERROR_ILLEGAL_SEEK, errmsg_illegal_seek);

String errmsg_directory_not_empty(ErrorArgs meta);
// args: FileOperation as int32, string path A, string path B
constexpr ErrorType ERROR_DIRECTORY_NOT_EMPTY = tr::errtype_from_string("tr::DIRECTORY_NOT_EMPTY");
TR_REGISTER_ERROR_TYPE(ERROR_DIRECTORY_NOT_EMPTY, errmsg_directory_not_empty);

// both errno.h and win32 do this bullshit
void _reset_os_errors();

ErrorType _trippin_error_from_errno();
#ifdef TR_OS_WINDOWS
ErrorType _trippin_error_from_win32();
#endif

// So spicy.
template<typename T>
requires(!std::is_reference_v<T>) // TODO fucking fix it
class [[nodiscard]] Result
{
	T _value{};
	Error _error;

public:
	using Type = T;

	constexpr Result(T val)
		: _value(val)
	{
	}
	constexpr Result(ErrorType type, ErrorArgs args)
		: _error({.args = args, .type = type})
	{
	}

	// Makes an error as well as passing arguments with questionable type safety
	template<typename... Args>
	constexpr Result(ErrorType type, Args... args)
	{
		_error.type = type;
		static_assert(sizeof...(Args) <= 16, "only 16 error args allowed");

		// a bit of evil fuckery to get the arg's type (fold expression©®™®™©)
		usize i = 0;
		(
			[&](auto arg) {
				if constexpr (std::is_same_v<decltype(arg), uint8>) {
					_error.args[i].u8 = arg;
				}
				else if constexpr (std::is_same_v<decltype(arg), uint16>) {
					_error.args[i].u16 = arg;
				}
				else if constexpr (std::is_same_v<decltype(arg), uint32>) {
					_error.args[i].u32 = arg;
				}
				else if constexpr (std::is_same_v<decltype(arg), uint64>) {
					_error.args[i].u64 = arg;
				}
				else if constexpr (std::is_same_v<decltype(arg), int8>) {
					_error.args[i].i8 = arg;
				}
				else if constexpr (std::is_same_v<decltype(arg), int16>) {
					_error.args[i].i16 = arg;
				}
				else if constexpr (std::is_same_v<decltype(arg), int32>) {
					_error.args[i].i32 = arg;
				}
				else if constexpr (std::is_same_v<decltype(arg), int64>) {
					_error.args[i].i64 = arg;
				}
				else if constexpr (std::is_same_v<decltype(arg), float32>) {
					_error.args[i].f32 = arg;
				}
				else if constexpr (std::is_same_v<decltype(arg), float64>) {
					_error.args[i].f64 = arg;
				}
				else if constexpr (std::is_same_v<decltype(arg), String>) {
					_error.args[i].str = arg;
				}
				// treat const char* as a string (which is probably what the user
				// meant anyway)
				else if constexpr (std::is_same_v<decltype(arg), const char*>) {
					_error.args[i].str = String{arg};
				}
				else if constexpr (std::is_pointer_v<decltype(arg)> &&
						   std::is_const_v<decltype(arg)>) {
					_error.args[i].const_ptr = arg;
				}
				else if constexpr (std::is_pointer_v<decltype(arg)>) {
					_error.args[i].ptr = arg;
				}
				else {
					static_assert(false, "unsupported type, use a pointer");
				}
				i++;
			}(args),
			...
		);
	}

	// If true, the result has a value. Else, it has an error.
	constexpr bool is_valid() const
	{
		return _error.type == 0;
	}

	// If true, the result has an error. Else, it has a value.
	constexpr bool is_invalid() const
	{
		return _error.type != 0;
	}

	T unwrap() const
	{
		if (is_invalid()) {
			tr::panic(
				"couldn't unwrap tr::Result<T>: %s",
				*tr::error_message(_error.type, _error.args)
			);
		}

		return _value;
	}

	Error unwrap_err() const
	{
		if (is_valid()) {
			tr::panic("couldn't unwrap tr::Result<T>'s error, as it's valid");
		}
		return _error;
	}

	// Shorthand for unwrap()
	T operator*() const
	{
		return unwrap();
	}

	// Similar to the `??`/null coalescing operator in modern languages
	const T value_or(const T other) const
	{
		return is_valid() ? unwrap() : other;
	}
};

// Result for when you don't need the result :D
template<>
class [[nodiscard]] Result<void>
{
	Error _error;

public:
	using Type = void;

	constexpr Result() {}
	constexpr Result(ErrorType type, ErrorArgs args)
		: _error({.args = args, .type = type})
	{
	}

	// Makes an error as well as passing arguments with questionable type safety
	template<typename... Args>
	constexpr Result(ErrorType type, Args... args)
	{
		_error.type = type;
		static_assert(sizeof...(Args) <= 16, "only 16 error args allowed");

		// a bit of evil fuckery to get the arg's type (fold expression©®™®™©)
		usize i = 0;
		(
			[&](auto arg) {
				if constexpr (std::is_same_v<decltype(arg), uint8>) {
					_error.args[i].u8 = arg;
				}
				else if constexpr (std::is_same_v<decltype(arg), uint16>) {
					_error.args[i].u16 = arg;
				}
				else if constexpr (std::is_same_v<decltype(arg), uint32>) {
					_error.args[i].u32 = arg;
				}
				else if constexpr (std::is_same_v<decltype(arg), uint64>) {
					_error.args[i].u64 = arg;
				}
				else if constexpr (std::is_same_v<decltype(arg), int8>) {
					_error.args[i].i8 = arg;
				}
				else if constexpr (std::is_same_v<decltype(arg), int16>) {
					_error.args[i].i16 = arg;
				}
				else if constexpr (std::is_same_v<decltype(arg), int32>) {
					_error.args[i].i32 = arg;
				}
				else if constexpr (std::is_same_v<decltype(arg), int64>) {
					_error.args[i].i64 = arg;
				}
				else if constexpr (std::is_same_v<decltype(arg), float32>) {
					_error.args[i].f32 = arg;
				}
				else if constexpr (std::is_same_v<decltype(arg), float64>) {
					_error.args[i].f64 = arg;
				}
				else if constexpr (std::is_same_v<decltype(arg), String>) {
					_error.args[i].str = arg;
				}
				else if constexpr (std::is_pointer_v<decltype(arg)> &&
						   std::is_const_v<decltype(arg)>) {
					_error.args[i].const_ptr = arg;
				}
				else if constexpr (std::is_pointer_v<decltype(arg)>) {
					_error.args[i].ptr = arg;
				}
				else {
					static_assert(false, "unsupported type, use a pointer");
				}
				i++;
			}(args),
			...
		);
	}

	// If true, it has a value (which is no value since this is a void result). Else, it has an
	// error.
	constexpr bool is_valid() const
	{
		return _error.type == 0;
	}

	// If true, it has an error. Else, it has a value (which is no value since this is a void
	// result).
	constexpr bool is_invalid() const
	{
		return _error.type != 0;
	}

	// Pretty much just asserts that it's valid :D
	void unwrap() const
	{
		if (is_invalid()) {
			tr::panic(
				"couldn't unwrap tr::Result<void>: %s",
				*tr::error_message(_error.type, _error.args)
			);
		}
	}

	void operator*() const
	{
		unwrap();
	}

	Error unwrap_err() const
	{
		if (is_valid()) {
			tr::panic("couldn't unwrap tr::Result<void>'s error, as it's valid");
		}
		return _error;
	}
};

}

#endif
