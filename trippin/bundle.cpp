/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/bundle.cpp
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

#include "bundle.hpp"

static tr::String field_type_to_str(tr::FieldType type)
{
	switch (type) {
		case tr::FieldType::VOID:     return "FieldType::VOID";
		case tr::FieldType::CUSTOM:   return "FieldType::CUSTOM";
		case tr::FieldType::MAYBE:    return "FieldType::MAYBE";
		case tr::FieldType::BOOL:     return "FieldType::BOOL";
		case tr::FieldType::CHAR:     return "FieldType::CHAR";
		case tr::FieldType::INT8:     return "FieldType::INT8";
		case tr::FieldType::INT16:    return "FieldType::INT16";
		case tr::FieldType::INT32:    return "FieldType::INT32";
		case tr::FieldType::INT64:    return "FieldType::INT64";
		case tr::FieldType::UINT8:    return "FieldType::UINT8";
		case tr::FieldType::UINT16:   return "FieldType::UINT16";
		case tr::FieldType::UINT32:   return "FieldType::UINT32";
		case tr::FieldType::UINT64:   return "FieldType::UINT64";
		case tr::FieldType::FLOAT32:  return "FieldType::FLOAT32";
		case tr::FieldType::FLOAT64:  return "FieldType::FLOAT64";
		case tr::FieldType::STRING:   return "FieldType::STRING";
		case tr::FieldType::ARRAY:    return "FieldType::ARRAY";
		case tr::FieldType::HASHMAP:  return "FieldType::HASHMAP";
	}
	return "invalid type";
}

// used when panicking
static tr::String field_to_str(tr::Field field)
{
	return tr::fmt(tr::scratchpad(),
		"tr::Field(name: '%s', type: %s, template type: %s, left type: %s, right type: %s)",
		field.name.buf(), field_type_to_str(field.base_type).buf(),
		field_type_to_str(field.template_type).buf(),
		field_type_to_str(field.pair_types.left).buf(),
		field_type_to_str(field.pair_types.right).buf()
	);
}

static void serialize_text_field();

// quite the mouthful
tr::Maybe<tr::Error> tr::__impl_serialize_text(const void* data, tr::Array<tr::Field> fields,
	tr::Writer& writer, tr::Arena& arena, tr::String format, tr::Vec3<uint32> version)
{
	// header
	TR_TRY(writer.println("@format: %s", format.buf()));
	TR_TRY(writer.println("@version: %u.%u.%u", version.x, version.y, version.z));
	TR_TRY(writer.println());

	// actual data :D
	for (auto [_, field] : fields) {
		switch (field.base_type) {
		case FieldType::VOID:
			tr::panic("error serializing bundle: can't serialize field of type void. ");
			break;
		case FieldType::CUSTOM:
			break;
		case FieldType::MAYBE:
			break;
		case FieldType::BOOL:
			break;
		case FieldType::CHAR:
			break;
		case FieldType::INT8:
			break;
		case FieldType::INT16:
			break;
		case FieldType::INT32:
			break;
		case FieldType::INT64:
			break;
		case FieldType::UINT8:
			break;
		case FieldType::UINT16:
			break;
		case FieldType::UINT32:
			break;
		case FieldType::UINT64:
			break;
		case FieldType::FLOAT32:
			break;
		case FieldType::FLOAT64:
			break;
		case FieldType::STRING:
			break;
		case FieldType::ARRAY:
			break;
		case FieldType::HASHMAP:
			break;
		}
	}
	return {};
}
