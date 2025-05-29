#include "libtrippin.hpp"

int main()
{
	tr::use_log_file("log.txt");
	tr::init();

	tr::log("sir");
	tr::liblog("sir");
	tr::warn("sir");
	tr::error("sir");
	// tr::assert(false, "trajìque");
	// tr::panic("AHHHHHHH");

	auto fuck2 = tr::Vec2<double>(1, 2) % tr::Vec2<double>(3, 4);
	tr::log("%f, %f", fuck2.x, fuck2.y);

	auto fuck3 = tr::Vec3<double>(1, 2, 3) % tr::Vec3<double>(4, 5, 6);
	tr::log("%f, %f, %f", fuck3.x, fuck3.y, fuck3.z);

	auto fuck4 = tr::Vec4<double>(1, 2, 3, 4) % tr::Vec4<double>(5, 6, 7, 8);
	tr::log("%f, %f, %f, %f", fuck4.x, fuck4.y, fuck4.z, fuck4.w);

	tr::free();
}
