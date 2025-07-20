/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/reflect.hpp
 * Reflection faffery
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

#ifndef _TRIPPIN_REFLECT_H
#define _TRIPPIN_REFLECT_H

#include "common.hpp"
#include "string.hpp"
#include "collection.hpp"
#include "math.hpp"

namespace tr {

// Yeah. This is useful for serialization and stuff.
enum class FieldType
{
	VOID, CUSTOM, MAYBE,
	BOOL, CHAR,
	INT8, INT16, INT32, INT64,
	UINT8, UINT16, UINT32, UINT64,
	FLOAT32, FLOAT64,
	STRING, ARRAY, HASHMAP,
};

// Represents a struct/class field
struct Field
{
	String name;
	usize offset;
	FieldType base_type;
	FieldType template_type;
	struct {
		FieldType left;
		FieldType right;
	} pair_types;
};

// Default implementation so C++ doesn't complain
template<typename T>
class TypeInfo
{
public:
	static constexpr String name()
	{
		static_assert(false, "using tr::TypeInfo without any implementation (read the docs for reflection)");
		return "";
	}

	static constexpr FieldType type()
	{
		static_assert(false, "using tr::TypeInfo without any implementation (read the docs for reflection)");
		return FieldType::VOID;
	}

	static constexpr Array<Field> fields()
	{
		static_assert(false, "using tr::TypeInfo without any implementation (read the docs for reflection)");
		return {};
	}

	static constexpr FieldType template_type()
	{
		static_assert(false, "using tr::TypeInfo without any implementation (read the docs for reflection)");
		return FieldType::VOID;
	}

	static constexpr FieldType left_template_type()
	{
		static_assert(false, "using tr::TypeInfo without any implementation (read the docs for reflection)");
		return FieldType::VOID;
	}

	static constexpr FieldType right_template_type()
	{
		static_assert(false, "using tr::TypeInfo without any implementation (read the docs for reflection)");
		return FieldType::VOID;
	}
};

// all the default implementations for TypeInfo

template<>
class TypeInfo<bool>
{
public:
	static constexpr String name()             { return "bool"; }
	static constexpr FieldType type()          { return FieldType::BOOL; }
	static constexpr Array<Field> fields()     { return {}; }

	static constexpr FieldType template_type()         { return FieldType::VOID; }
	static constexpr FieldType left_template_type()    { return FieldType::VOID; }
	static constexpr FieldType right_template_type()   { return FieldType::VOID; }
};

template<>
class TypeInfo<char>
{
public:
	static constexpr String name()             { return "char"; }
	static constexpr FieldType type()          { return FieldType::CHAR; }
	static constexpr Array<Field> fields()     { return {}; }

	static constexpr FieldType template_type()         { return FieldType::VOID; }
	static constexpr FieldType left_template_type()    { return FieldType::VOID; }
	static constexpr FieldType right_template_type()   { return FieldType::VOID; }
};

template<>
class TypeInfo<int8>
{
public:
	static constexpr String name()             { return "int8"; }
	static constexpr FieldType type()          { return FieldType::INT8; }
	static constexpr Array<Field> fields()     { return {}; }

	static constexpr FieldType template_type()         { return FieldType::VOID; }
	static constexpr FieldType left_template_type()    { return FieldType::VOID; }
	static constexpr FieldType right_template_type()   { return FieldType::VOID; }
};

template<>
class TypeInfo<int16>
{
public:
	static constexpr String name()             { return "int16"; }
	static constexpr FieldType type()          { return FieldType::INT16; }
	static constexpr Array<Field> fields()     { return {}; }

	static constexpr FieldType template_type()         { return FieldType::VOID; }
	static constexpr FieldType left_template_type()    { return FieldType::VOID; }
	static constexpr FieldType right_template_type()   { return FieldType::VOID; }
};

template<>
class TypeInfo<int32>
{
public:
	static constexpr String name()             { return "int32"; }
	static constexpr FieldType type()          { return FieldType::INT32; }
	static constexpr Array<Field> fields()     { return {}; }

	static constexpr FieldType template_type()         { return FieldType::VOID; }
	static constexpr FieldType left_template_type()    { return FieldType::VOID; }
	static constexpr FieldType right_template_type()   { return FieldType::VOID; }
};

template<>
class TypeInfo<int64>
{
public:
	static constexpr String name()             { return "int64"; }
	static constexpr FieldType type()          { return FieldType::INT64; }
	static constexpr Array<Field> fields()     { return {}; }

	static constexpr FieldType template_type()         { return FieldType::VOID; }
	static constexpr FieldType left_template_type()    { return FieldType::VOID; }
	static constexpr FieldType right_template_type()   { return FieldType::VOID; }
};

template<>
class TypeInfo<uint8>
{
public:
	static constexpr String name()             { return "uint8"; }
	static constexpr FieldType type()          { return FieldType::UINT8; }
	static constexpr Array<Field> fields()     { return {}; }

	static constexpr FieldType template_type()         { return FieldType::VOID; }
	static constexpr FieldType left_template_type()    { return FieldType::VOID; }
	static constexpr FieldType right_template_type()   { return FieldType::VOID; }
};

template<>
class TypeInfo<uint16>
{
public:
	static constexpr String name()             { return "uint16"; }
	static constexpr FieldType type()          { return FieldType::UINT16; }
	static constexpr Array<Field> fields()     { return {}; }

	static constexpr FieldType template_type()         { return FieldType::VOID; }
	static constexpr FieldType left_template_type()    { return FieldType::VOID; }
	static constexpr FieldType right_template_type()   { return FieldType::VOID; }
};

template<>
class TypeInfo<uint32>
{
public:
	static constexpr String name()             { return "uint32"; }
	static constexpr FieldType type()          { return FieldType::UINT32; }
	static constexpr Array<Field> fields()     { return {}; }

	static constexpr FieldType template_type()         { return FieldType::VOID; }
	static constexpr FieldType left_template_type()    { return FieldType::VOID; }
	static constexpr FieldType right_template_type()   { return FieldType::VOID; }
};

template<>
class TypeInfo<uint64>
{
public:
	static constexpr String name()             { return "uint64"; }
	static constexpr FieldType type()          { return FieldType::UINT64; }
	static constexpr Array<Field> fields()     { return {}; }

	static constexpr FieldType template_type()         { return FieldType::VOID; }
	static constexpr FieldType left_template_type()    { return FieldType::VOID; }
	static constexpr FieldType right_template_type()   { return FieldType::VOID; }
};

template<>
class TypeInfo<float32>
{
public:
	static constexpr String name()             { return "float32"; }
	static constexpr FieldType type()          { return FieldType::FLOAT32; }
	static constexpr Array<Field> fields()     { return {}; }

	static constexpr FieldType template_type()         { return FieldType::VOID; }
	static constexpr FieldType left_template_type()    { return FieldType::VOID; }
	static constexpr FieldType right_template_type()   { return FieldType::VOID; }
};

template<>
class TypeInfo<float64>
{
public:
	static constexpr String name()             { return "float64"; }
	static constexpr FieldType type()          { return FieldType::FLOAT64; }
	static constexpr Array<Field> fields()     { return {}; }

	static constexpr FieldType template_type()         { return FieldType::VOID; }
	static constexpr FieldType left_template_type()    { return FieldType::VOID; }
	static constexpr FieldType right_template_type()   { return FieldType::VOID; }
};

template<>
class TypeInfo<String>
{
public:
	static constexpr String name()             { return "tr::String"; }
	static constexpr FieldType type()          { return FieldType::STRING; }
	static constexpr Array<Field> fields()     { return {}; }

	static constexpr FieldType template_type()         { return FieldType::CHAR; }
	static constexpr FieldType left_template_type()    { return FieldType::VOID; }
	static constexpr FieldType right_template_type()   { return FieldType::VOID; }
};

template<typename T>
class TypeInfo<Array<T>>
{
public:
	// TODO should this include T's name?
	// like tr::fmt("tr::Array<%s>", TypeInfo<T>::name())
	// though tr::fmt isn't constexpr
	static constexpr String name()             { return "tr::Array<T>"; }
	static constexpr FieldType type()          { return FieldType::ARRAY; }
	static constexpr Array<Field> fields()     { return {}; }

	static constexpr FieldType template_type()         { return FieldType::VOID; }
	static constexpr FieldType left_template_type()    { return FieldType::VOID; }
	static constexpr FieldType right_template_type()   { return FieldType::VOID; }
};

template<typename T>
class TypeInfo<Maybe<T>>
{
public:
	static constexpr String name()             { return "tr::Maybe<T>"; }
	static constexpr FieldType type()          { return FieldType::MAYBE; }
	static constexpr Array<Field> fields()     { return {}; }

	static constexpr FieldType template_type()         { return FieldType::VOID; }
	static constexpr FieldType left_template_type()    { return FieldType::VOID; }
	static constexpr FieldType right_template_type()   { return FieldType::VOID; }
};

template<typename K, typename V>
class TypeInfo<HashMap<K, V>>
{
public:
	static constexpr String name()             { return "tr::HashMap<K, V>"; }
	static constexpr FieldType type()          { return FieldType::HASHMAP; }
	static constexpr Array<Field> fields()     { return {}; }

	static constexpr FieldType template_type()         { return FieldType::VOID; }
	static constexpr FieldType left_template_type()    { return TypeInfo<K>::type(); }
	static constexpr FieldType right_template_type()   { return TypeInfo<V>::type(); }
};

// macros for custom types
#define TR_START_TYPE_INFO(T) \
	/* shut up */ \
	TR_GCC_IGNORE_WARNING(-Wextra-qualification) \
	template<> \
	class tr::TypeInfo<T> \
	{ \
	public: \
		/* so it can be used later by TR_FIELD */ \
		using Type = T; \
		static constexpr tr::String name()                   { return #T; } \
		static constexpr tr::FieldType type()                { return tr::FieldType::CUSTOM; } \
		static constexpr tr::FieldType template_type()       { return tr::FieldType::VOID; } \
		static constexpr tr::FieldType left_template_type()  { return tr::FieldType::VOID; } \
		static constexpr tr::FieldType right_template_type() { return tr::FieldType::VOID; } \
		\
		static constexpr tr::Array<tr::Field> fields() \
		{ \
			return {

#define TR_END_TYPE_INFO() \
			}; \
		} \
	}; \
	TR_GCC_RESTORE()

// the macros that go in between that
// god help us all
#define TR_FIELD(F) \
	/* name, offset, type */ \
	tr::Field{#F, offsetof(Type, F), tr::TypeInfo<decltype(Type::F)>::type(), \
	/* some additional crap we don't use */ \
	tr::FieldType::VOID, {tr::FieldType::VOID, tr::FieldType::VOID}},

#define TR_FIELD_ARRAY(F) \
	/* name, offset, type */ \
	tr::Field{#F, offsetof(Type, F), tr::TypeInfo<decltype(Type::F)>::type(), \
	/* evil fuckery to get the array's type */ \
	tr::TypeInfo<decltype(Type::F)::__T>::type(), {tr::FieldType::VOID, tr::FieldType::VOID}},

#define TR_FIELD_MAYBE(F) \
	/* name, offset, type */ \
	tr::Field{#F, offsetof(Type, F), tr::TypeInfo<decltype(Type::F)>::type(), \
	/* evil fuckery to get the array's type */ \
	tr::TypeInfo<decltype(Type::F)::__T>::type(), {tr::FieldType::VOID, tr::FieldType::VOID}},

#define TR_FIELD_HASHMAP(F) \
	/* name, offset, type */ \
	tr::Field{#F, offsetof(Type, F), tr::TypeInfo<decltype(Type::F)>::type(), \
	/* evil fuckery to get the hashmap's type */ \
	tr::FieldType::VOID, {tr::TypeInfo<decltype(Type::F)::__K>::type(), \
		tr::TypeInfo<decltype(Type::F)::__V>::type()}},

// more definitions for everything ever :DDDDDDDDD
// TODO should vectors use template_type()?
TR_START_TYPE_INFO(tr::Vec2<int8>)    TR_FIELD(x) TR_FIELD(y)                         TR_END_TYPE_INFO()
TR_START_TYPE_INFO(tr::Vec2<int16>)   TR_FIELD(x) TR_FIELD(y)                         TR_END_TYPE_INFO()
TR_START_TYPE_INFO(tr::Vec2<int32>)   TR_FIELD(x) TR_FIELD(y)                         TR_END_TYPE_INFO()
TR_START_TYPE_INFO(tr::Vec2<int64>)   TR_FIELD(x) TR_FIELD(y)                         TR_END_TYPE_INFO()
TR_START_TYPE_INFO(tr::Vec2<uint8>)   TR_FIELD(x) TR_FIELD(y)                         TR_END_TYPE_INFO()
TR_START_TYPE_INFO(tr::Vec2<uint16>)  TR_FIELD(x) TR_FIELD(y)                         TR_END_TYPE_INFO()
TR_START_TYPE_INFO(tr::Vec2<uint32>)  TR_FIELD(x) TR_FIELD(y)                         TR_END_TYPE_INFO()
TR_START_TYPE_INFO(tr::Vec2<uint64>)  TR_FIELD(x) TR_FIELD(y)                         TR_END_TYPE_INFO()
TR_START_TYPE_INFO(tr::Vec2<float32>) TR_FIELD(x) TR_FIELD(y)                         TR_END_TYPE_INFO()
TR_START_TYPE_INFO(tr::Vec2<float64>) TR_FIELD(x) TR_FIELD(y)                         TR_END_TYPE_INFO()
TR_START_TYPE_INFO(tr::Vec3<int8>)    TR_FIELD(x) TR_FIELD(y) TR_FIELD(z)             TR_END_TYPE_INFO()
TR_START_TYPE_INFO(tr::Vec3<int16>)   TR_FIELD(x) TR_FIELD(y) TR_FIELD(z)             TR_END_TYPE_INFO()
TR_START_TYPE_INFO(tr::Vec3<int32>)   TR_FIELD(x) TR_FIELD(y) TR_FIELD(z)             TR_END_TYPE_INFO()
TR_START_TYPE_INFO(tr::Vec3<int64>)   TR_FIELD(x) TR_FIELD(y) TR_FIELD(z)             TR_END_TYPE_INFO()
TR_START_TYPE_INFO(tr::Vec3<uint8>)   TR_FIELD(x) TR_FIELD(y) TR_FIELD(z)             TR_END_TYPE_INFO()
TR_START_TYPE_INFO(tr::Vec3<uint16>)  TR_FIELD(x) TR_FIELD(y) TR_FIELD(z)             TR_END_TYPE_INFO()
TR_START_TYPE_INFO(tr::Vec3<uint32>)  TR_FIELD(x) TR_FIELD(y) TR_FIELD(z)             TR_END_TYPE_INFO()
TR_START_TYPE_INFO(tr::Vec3<uint64>)  TR_FIELD(x) TR_FIELD(y) TR_FIELD(z)             TR_END_TYPE_INFO()
TR_START_TYPE_INFO(tr::Vec3<float32>) TR_FIELD(x) TR_FIELD(y) TR_FIELD(z)             TR_END_TYPE_INFO()
TR_START_TYPE_INFO(tr::Vec3<float64>) TR_FIELD(x) TR_FIELD(y) TR_FIELD(z)             TR_END_TYPE_INFO()
TR_START_TYPE_INFO(tr::Vec4<int8>)    TR_FIELD(x) TR_FIELD(y) TR_FIELD(z) TR_FIELD(w) TR_END_TYPE_INFO()
TR_START_TYPE_INFO(tr::Vec4<int16>)   TR_FIELD(x) TR_FIELD(y) TR_FIELD(z) TR_FIELD(w) TR_END_TYPE_INFO()
TR_START_TYPE_INFO(tr::Vec4<int32>)   TR_FIELD(x) TR_FIELD(y) TR_FIELD(z) TR_FIELD(w) TR_END_TYPE_INFO()
TR_START_TYPE_INFO(tr::Vec4<int64>)   TR_FIELD(x) TR_FIELD(y) TR_FIELD(z) TR_FIELD(w) TR_END_TYPE_INFO()
TR_START_TYPE_INFO(tr::Vec4<uint8>)   TR_FIELD(x) TR_FIELD(y) TR_FIELD(z) TR_FIELD(w) TR_END_TYPE_INFO()
TR_START_TYPE_INFO(tr::Vec4<uint16>)  TR_FIELD(x) TR_FIELD(y) TR_FIELD(z) TR_FIELD(w) TR_END_TYPE_INFO()
TR_START_TYPE_INFO(tr::Vec4<uint32>)  TR_FIELD(x) TR_FIELD(y) TR_FIELD(z) TR_FIELD(w) TR_END_TYPE_INFO()
TR_START_TYPE_INFO(tr::Vec4<uint64>)  TR_FIELD(x) TR_FIELD(y) TR_FIELD(z) TR_FIELD(w) TR_END_TYPE_INFO()
TR_START_TYPE_INFO(tr::Vec4<float32>) TR_FIELD(x) TR_FIELD(y) TR_FIELD(z) TR_FIELD(w) TR_END_TYPE_INFO()
TR_START_TYPE_INFO(tr::Vec4<float64>) TR_FIELD(x) TR_FIELD(y) TR_FIELD(z) TR_FIELD(w) TR_END_TYPE_INFO()
TR_START_TYPE_INFO(tr::Color)         TR_FIELD(r) TR_FIELD(g) TR_FIELD(b) TR_FIELD(a) TR_END_TYPE_INFO()

}

#endif
