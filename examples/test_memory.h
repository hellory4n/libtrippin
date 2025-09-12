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
	tr::Arena arena;
	TR_DEFER(arena.free());

	// arenas are infinite
	// allocate as many as you want!
	auto* crap = static_cast<CrapStruct*>(arena.alloc(sizeof(CrapStruct)));
	// nicer wrapper, it even supports passing arguments to the constructor
	CrapStruct& crap2 = arena.make_ref<CrapStruct>();

	// you can also allocate arrays
	// tr::scratchpad() is a temporary arena (like a sane alloca)
	tr::Array<int64> array(tr::scratchpad(), {1, 2, 3, 4, 5});
	array.add(6);
	for (auto [i, num] : array) {
		tr::log("array[%zu] = %li", i, num);
	}
}
