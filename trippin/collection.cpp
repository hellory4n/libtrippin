/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/collection.cpp
 * Additional collection types
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

// TODO the header thing is longer than the actual code, consider using this file

#include "trippin/collection.h"

constexpr uint64 FNV_OFFSET_BASIS = 0xcbf29ce484222325;
// IM IN MY PRIME™ AND THIS AINT EVEN FINAL FORM
constexpr uint64 FNV_PRIME = 0x100000001b3;

uint64 tr::hash(tr::Array<const uint8> array)
{
	uint64 hash = FNV_OFFSET_BASIS;

	for (auto [_, byte] : array) {
		hash ^= byte;
		hash *= FNV_PRIME;
	}

	return hash;
}
