#include "libtrippin.hpp"
#include <cstdio>

int main()
{
	tr::use_log_file("log.txt");
	tr::init();

	tr::log("sir");
	tr::info("sir");
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
	maybe = 57;
	if (maybe.is_valid()) {
		tr::log("perchance %i", maybe.unwrap());
	}

	auto it_depends_yknow = tr::Either<int32, bool>(64);
	if (it_depends_yknow.is_left()) {
		tr::log("left: %i", it_depends_yknow.left());
	}
	it_depends_yknow.right();

	auto should_i_call_it_both = tr::Pair<int64, float64>(1, 2.2);
	tr::log("pair left: %li, pair right %f", should_i_call_it_both.left, should_i_call_it_both.right);

	tr::MaybeRef<tr::Arena> arena = new tr::Arena(tr::kb_to_bytes(2));
	// arena.prealloc(tr::mb_to_bytes(1));
	struct ComicallyLargeStruct {
		uint8 man[1024];
	};
	struct EvenLargerStruct {
		uint8 man[4096];
	};
	// if it works it won't segfault
	auto* sig = arena->alloc<ComicallyLargeStruct>();
	auto* ma = arena->alloc<EvenLargerStruct>();
	tr::log("it did allocate");
	sig->man[12] = 221;
	ma->man[35] = 54;
	tr::log("look %i", sig->man[12]);
	tr::log("oh look at what you've done now to me %i", ma->man[35]);

	auto random = tr::Random();
	tr::log("sogmarand %f", random.next(1.0, 1.4));
	tr::log("sogmarand %li", random.next<int64>(1, 69'000'000'000));

	auto lerp_a = tr::Vec2<int64>(1, 2);
	auto lerp_b = tr::Vec2<int64>(15393, 2376);
	tr::Vec2<int64> lerp = tr::lerp(lerp_a, lerp_b, 0.5);
	tr::log("lerp vec2<int64>: %li, %li", lerp.x, lerp.y);

	int64 items[] = {11, 22, 33, 44, 55};
	tr::Array<int64> array(arena, items, sizeof(items) / sizeof(int64));
	array.add(66);
	for (tr::ArrayItem<int64> item : array) {
		tr::log("array[%zu] = %li", item.i, item.val);
	}

	tr::String str = "sigma";
	tr::log("str: %s (length %zu)", str.buffer(), str.length());

	tr::String maballs = tr::sprintf(arena, 256, "%s balls", str.buffer());
	tr::log("%s", maballs.buffer());

	tr::assert(str == "sigma", "strings are busted");
	tr::assert(str != "ballshshjs", "strings are busted");
	tr::assert(str != "sigmaaaa pelotas", "strings are busted");
	tr::assert(str.substr(arena, 1, 3) == "igm", "strings are busted");
	tr::Array<usize> sigma = tr::String("sigmysigmy").find(arena, "ig");
	tr::assert(sigma.length() == 2, "strings are busted");
	tr::String sigmaa = tr::String("figma").concat(arena, " balls");
	tr::assert(sigmaa == "figma balls", "strings are busted");
	tr::assert(sigmaa.starts_with("figm"), "strings are busted");
	tr::assert(sigmaa.ends_with("alls"), "strings are busted");

	tr::assert(tr::String("/path/to/file.txt").file(arena) == "file.txt", "paths are busted");
	tr::assert(tr::String("/path/to/file.txt").directory(arena) == "/path/to", "paths are busted");
	tr::assert(tr::String("/path/to/teapot.blend.1").extension(arena) == ".blend.1", "paths are busted");
	tr::assert(tr::String("app://sigma").is_absolute(), "paths are busted");
	tr::assert(tr::String("C:\\sigma").is_absolute(), "paths are busted");
	tr::assert(!tr::String("sigma").is_absolute(), "paths are busted");
	tr::assert(!tr::String("./sigma").is_absolute(), "paths are busted");
	tr::assert(tr::String("~/sigma").is_absolute(), "paths are busted");

	// man
	// tr::MemoryInfo memory = tr::get_memory_info();
	// couldn't be bothered to print shit so i just read it in gdb
	// tr::panic("lol");

	tr::free();
}
