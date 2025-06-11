#include "libtrippin.hpp"

int main(void)
{
	// temporary string
	tr::String str1 = "hi mom";
	tr::log("%s", str1.buffer());

	// arena string
	tr::Ref<tr::Arena> arena = new tr::Arena(tr::kb_to_bytes(64));
	tr::String str2(arena, "a string", sizeof("a string"));

	// you can do formatting too
	tr::String str3 = tr::sprintf(arena, 32, "hi %s", "mom");
}
