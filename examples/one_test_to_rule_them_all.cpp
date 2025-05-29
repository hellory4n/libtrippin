#include "libtrippin.hpp"
#include <cstdio>

int main()
{
	tr::init("log.txt");

	tr::log("sir");
	tr::liblog("sir");
	tr::warn("sir");
	tr::error("sir");
	// tr::assert(false, "trajìque");
	tr::panic("AHHHHHHH");

	tr::free();
}
