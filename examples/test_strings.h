#pragma once
#include <trippin/string.h>

void test_strings()
{
	tr::ScratchArena scratch{};
	TR_DEFER(scratch.free());

	// temporary string
	tr::String str = "hi mom";
	tr::log("%s", *str);

	// arena string
	tr::String str2{scratch, "a string", sizeof("a string")};

	// you can do formatting too
	tr::String str3 = tr::fmt(scratch, "hi %s", "mom");

	// for short-lived strings you can use tr::tmp_fmt
	tr::String str4 = tr::tmp_fmt("hi %s", "mom");
}
