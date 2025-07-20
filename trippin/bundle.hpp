/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/bundle.hpp
 * Most massive file format
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

#ifndef _TRIPPIN_BUNDLE_H
#define _TRIPPIN_BUNDLE_H

#include "common.hpp"
#include "memory.hpp"
#include "iofs.hpp"
#include "error.hpp"
#include "string.hpp"
#include "math.hpp"
#include "reflect.hpp"

namespace tr {

// default implementation
template<typename T>
class Bundle
{
	static constexpr String format = "trippin/bundle";
	static constexpr Vec3<uint32> version = {1, 0, 0};
};

// probably overkill i dont care
// variadic so e.g. {1, 0, 0} doesn't break everything
#define TR_BUNDLE_FORMAT(Type, Name, ...) \
	TR_GCC_IGNORE_WARNING(-Wextra-qualification) \
	template<> \
	class tr::Bundle<Type> \
	{ \
		static constexpr tr::String format = Name; \
		static constexpr tr::Vec3<uint32> version = __VA_ARGS__; \
	}; \
	TR_GCC_RESTORE()

// internal don't use, use `tr::serialize_text_bundle`
Maybe<Error> __impl_serialize_text(const void* data, Array<Field> fields, Writer& writer, Arena& arena,
	String format, Vec3<uint32> version);

// Serializes data into TBOB (usually a .tbob file). The type you're serializing must have a
// `tr::TypeInfo<T>` as well as a `tr::Bundle<T>`
template<typename T>
tr::Result<void, tr::Error> serialize_text_bundle(tr::Arena& arena, tr::Writer& writer, const T& data)
{
	// :(
	tr::Maybe<tr::Error> error = tr::__impl_serialize_text(&data, TypeInfo<T>::fields(),
		writer, arena, Bundle<T>::format, Bundle<T>::version);

	if (error.is_valid()) {
		return error.unwrap();
	}
	return {};
}

}

#endif
