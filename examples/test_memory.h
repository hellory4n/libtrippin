#pragma once
#include <trippin/log.h>
#include <trippin/memory.h>

#include "trippin/common.h"

struct CrapStruct
{
	uint8 man[128];
};

void test_memory()
{
	tr::Arena arena{};
	TR_DEFER(arena.free());

	// arenas are infinite
	// allocate as many as you want!
	auto* crap = arena.alloc<CrapStruct*>(sizeof(CrapStruct));
	// nicer wrapper, it even supports passing arguments to the constructor
	CrapStruct& crap2 = arena.make_ref<CrapStruct>();

	// tr::ScratchArena can be used for temporary allocations (like a sane alloc)
	tr::ScratchArena scratch{};
	TR_DEFER(scratch.free());

	// you can also allocate arrays
	tr::Array<int64> array{
		scratch, {1, 2, 3, 4, 5}
	};
	array.add(6);
	for (auto [i, num] : array) {
		tr::log("array[%zu] = %li", i, num);
	}
}
