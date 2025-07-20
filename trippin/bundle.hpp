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
#include "reflect.hpp"

namespace tr {

Maybe<Error> __impl_serialize(const void* out, Array<Field> fields, Writer& writer, Arena& arena);

template<typename T>
tr::Result<void, tr::Error> serialize_bundle(tr::Arena& arena, tr::Writer& writer, const T& data)
{
	tr::Maybe<tr::Error> error = tr::__impl_serialize(&data, tr::TypeInfo<T>::fields(), writer, arena);
	if (error.is_valid()) {
		return error.unwrap();
	}
	return {};
}

}

#endif
