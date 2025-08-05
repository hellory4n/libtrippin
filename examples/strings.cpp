#include <trippin/string.h>

int main()
{
	// temporary string
	tr::String str = "hi mom";
	tr::log("%s", *str);

	// arena string
	tr::String str2(tr::scratchpad(), "a string", sizeof("a string"));

	// you can do formatting too
	tr::String str3 = tr::fmt(tr::scratchpad(), "hi %s", "mom");
}
