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

	auto fuck2 = tr::Vec2<float64>(1, 2) % tr::Vec2<float64>(3, 4);
	tr::log("%f, %f", fuck2.x, fuck2.y);

	auto fuck3 = tr::Vec3<float64>(1, 2, 3) % tr::Vec3<float64>(4, 5, 6);
	tr::log("%f, %f, %f", fuck3.x, fuck3.y, fuck3.z);

	auto fuck4 = tr::Vec4<float64>(1, 2, 3, 4) % tr::Vec4<float64>(5, 6, 7, 8);
	tr::log("%f, %f, %f, %f", fuck4.x, fuck4.y, fuck4.z, fuck4.w);

	tr::Maybe<int32> maybe;
	if (!maybe.is_valid()) {
		tr::log("nope");
	}
	// maybe.unwrap();
	maybe = tr::Maybe<int32>(57);
	if (maybe.is_valid()) {
		tr::log("perchance %i", *maybe.unwrap());
	}

	auto it_depends_yknow = tr::Either<int32, bool>(64);
	if (it_depends_yknow.is_left()) {
		tr::log("left: %i", it_depends_yknow.left());
	}
	it_depends_yknow.right();

	auto should_i_call_it_both = tr::Pair<int64, float64>(1, 2.2);
	tr::log("pair left: %li, pair right %f", should_i_call_it_both.left, should_i_call_it_both.right);

	tr::free();
}
