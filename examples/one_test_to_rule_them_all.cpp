#include <stdio.h>
#include <trippin/common.hpp>
#include <trippin/log.hpp>
#include <trippin/math.hpp>
#include <trippin/memory.hpp>
#include <trippin/string.hpp>
#include <trippin/collection.hpp>
#include <trippin/iofs.hpp>

// TODO use actual tests you dumbass

namespace test {
	static void logging();
	static void memory();
	static void arrays();
	static void strings();
	static void hashmaps();
	static void filesystem();
	static void all();
}

static void test::logging()
{
	tr::log("\n==== LOGGING ====");

	tr::log("sir");
	tr::info("sir");
	tr::warn("sir");
	tr::error("sir");
	// tr::assert(false, "trajìque");
	// tr::panic("AHHHHHHH");

	tr::log("S%sa (formatted arguments)", "igm");

	tr::call_on_quit([&]() -> void {
		tr::log("CUSTOM FUNCTION THAT RUNS ON QUIT/PANIC??? SCRUMPTIOUS");
	});
}

static void test::memory()
{
	tr::log("\n==== MEMORY ====");

	tr::Arena arena;
	tr::Vec3<float32>& vecma3 = arena.make<tr::Vec3<float32>>(1, 2, 3);
	tr::log("vecma3 %f, %f, %f", vecma3.x, vecma3.y, vecma3.z);

	struct MaBalls {
		uint8 waste[tr::kb_to_bytes(1)];
		MaBalls() { tr::log("MaBalls created"); }
		~MaBalls() { tr::log("MaBalls deleted"); }
	};

	auto& sig = arena.make<MaBalls>();
	sig.waste[37] = 'm';

	arena.reset();
	TR_ASSERT_MSG(sig.waste[37] == 0, "it didn't reset properly :(");
	arena.alloc(tr::mb_to_bytes(1));

	tr::log("capacity: %zu KB, allocated: %zu KB",
		tr::bytes_to_kb(arena.capacity()), tr::bytes_to_kb(arena.allocated())
	);
}

static void test::arrays()
{
	tr::log("\n==== ARRAYS ====");

	tr::Array<int64> array(tr::scratchpad, {1, 2, 3, 4, 5});
	array.add(66);
	for (tr::ArrayItem<int64> item : array) {
		tr::log("array[%zu] = %li", item.i, item.val);
	}

	TR_ASSERT(!array.try_get(893463).is_valid());
	TR_ASSERT(array.try_get(1).is_valid());
}

static void test::strings()
{
	tr::log("\n==== STRINGS ====");

	tr::String str = "sigma";
	tr::log("str: %s (length %zu)", str.buf(), str.len());

	tr::String maballs = tr::sprintf(tr::scratchpad, "%s balls", str.buf());
	TR_ASSERT(maballs == "sigma balls");
	tr::log("%s", maballs.buf());

	TR_ASSERT(str == "sigma");
	TR_ASSERT(str != "ballshshjs");
	TR_ASSERT(str != "sigmaaaa pelotas");
	TR_ASSERT(str.substr(tr::scratchpad, 1, 3) == "igm");
	tr::Array<usize> sigma = tr::String("sigmysigmy").find(tr::scratchpad, "ig");
	TR_ASSERT(sigma.len() == 2);
	tr::String sigmaa = tr::String("figma").concat(tr::scratchpad, " balls");
	TR_ASSERT(sigmaa == "figma balls");
	TR_ASSERT(sigmaa.starts_with("figm"));
	TR_ASSERT(sigmaa.ends_with("alls"));

	TR_ASSERT(tr::String("/path/to/file.txt").file(tr::scratchpad) == "file.txt");
	TR_ASSERT(tr::String("/path/to/file.txt").directory(tr::scratchpad) == "/path/to");
	TR_ASSERT(tr::String("/path/to/teapot.blend.1").extension(tr::scratchpad) == ".blend.1");
	TR_ASSERT(tr::String("app://sigma").is_absolute());
	TR_ASSERT(tr::String("C:\\sigma").is_absolute());
	TR_ASSERT(!tr::String("sigma").is_absolute());
	TR_ASSERT(!tr::String("./sigma").is_absolute());
	TR_ASSERT(tr::String("~/sigma").is_absolute());

	// split string :)
	tr::String strma = "crap,shit,fuck,balls";
	tr::Array<tr::String> splitma = strma.split(tr::scratchpad, ',');
	for (auto c : splitma) {
		tr::log("split[%zu] = %s", c.i, c.val.buf());
	}
}

static void test::hashmaps()
{
	tr::log("\n==== HASHMAPS ====");

	tr::HashMap<tr::String, tr::String> hashma(tr::scratchpad);
	hashma["Sigma"] = "balls!";
	tr::log("hashma[\"Sigma\"] = \"%s\"", hashma["Sigma"].buf());

	// check collsiions
	tr::HashMapSettings<tr::String> settings{};
	settings.load_factor = 0.1;
	settings.initial_capacity = 4;
	settings.hash_func = [](const tr::String&) -> uint64 {
		return 68;
	};

	tr::HashMap<tr::String, tr::String> hashmaballs(tr::scratchpad, settings);
	// this also resizes bcuz the load factor is 0.1 and the capacity is 4 (comically small)
	hashmaballs["Sigma"] = "balls!";
	hashmaballs["Balls"] = "sigma!";
	hashmaballs["oh no it go it gone bye bye (bye)"] = "ball. one";
	hashmaballs["oSgmggg"] = "ball. one ball.";
	hashmaballs.remove("oh no it go it gone bye bye (bye)");

	// iterator
	for (auto item : hashmaballs) {
		tr::log("hashmaballs[%s] = \"%s\"", item.left.buf(), item.right.buf());
	}
	tr::log("length %zu, capacity %zu", hashmaballs.len(), hashmaballs.cap());
}

static void test::filesystem()
{
	tr::log("\n==== FILESYSTEM ====");

	// we don't care about these ones failing :)
	// it'll only succeed if it failed last time
	tr::remove_file("fucker.txt");
	tr::remove_file("fuckoffman.txt");

	// so much .unwrap() it looks like rust
	// i want it to crash if something goes wrong tho so that's why
	tr::File& wf = *tr::File::open(tr::scratchpad, "fucker.txt", tr::FileMode::WRITE_TEXT).unwrap();
	wf.write_string("Crap crappington.\nother line", false);
	wf.close();

	tr::File& rf = *tr::File::open(tr::scratchpad, "fucker.txt", tr::FileMode::READ_TEXT).unwrap();
	tr::String line1 = rf.read_line(tr::scratchpad).unwrap();
	tr::String line2 = rf.read_line(tr::scratchpad).unwrap();
	tr::log("line 1: %s; line 2: %s", line1.buf(), line2.buf());
	rf.close();

	tr::std_out.write_string("EVIL PRINTF FROM LIBTRIPPIN\n", false).unwrap();
	tr::std_out.write_string("please input some fucking bullshit: ", false).unwrap();
	tr::String line = tr::std_in.read_line(tr::scratchpad).unwrap();
	tr::std_out.write_string("the fucking bullshit: ", false).unwrap();
	tr::std_out.write_string(line, false).unwrap();
	tr::std_out.write_string("\n", false).unwrap();

	// help.
	// tr::move_file("fucker.txt", "fuckoffman.txt").unwrap();
	// TR_ASSERT(tr::file_exists("fuckoffman.txt"));
	// tr::remove_file("fuckoffman.txt").unwrap();
	tr::remove_file("fucker.txt");
}

static void test::all()
{
	test::logging();
	test::memory();
	test::arrays();
	test::strings();
	test::hashmaps();
	test::filesystem();
}

int main(int argc, char* argv[])
{
	tr::use_log_file("log.txt");
	tr::init();

	if (argc >= 2) {
		tr::String arg = argv[1];
		if (arg == "--logging") {
			test::logging();
		}
		else if (arg == "-- memory") {
			test::memory();
		}
		else if (arg == "--array") {
			test::arrays();
		}
		else if (arg == "--string") {
			test::strings();
		}
		else if (arg == "--hashmap") {
			test::hashmaps();
		}
		else if (arg == "--filesystem") {
			test::filesystem();
		}
		else if (arg == "--all") {
			test::all();
		}
		else {
			printf("The libtrippin tester 5000™\n");
			printf("Options:\n");
			printf("- --logging: Test logging\n");
			printf("- --memory: Test memory\n");
			printf("- --array: Test arrays\n");
			printf("- --string: Test strings\n");
			printf("- --hashmap: Test hashmaps\n");
			printf("- --filesystem: Test filesystem\n");
			printf("- --all: Test everything\n");
		}
	}
	else {
		printf("No options given, assuming --all\n");
		test::all();
	}

	tr::free();
	return 0;

	// tr::log("sir");
	// tr::info("sir");
	// tr::warn("sir");
	// tr::error("sir");
	// // tr::assert(false, "trajìque");
	// // tr::panic("AHHHHHHH");

	// auto fuck2 = tr::Vec2<float64>(1, 2) % tr::Vec2<float64>(3, 4);
	// tr::log("%f, %f", fuck2.x, fuck2.y);

	// auto fuck3 = tr::Vec3<float64>(1, 2, 3) % tr::Vec3<float64>(4, 5, 6);
	// tr::log("%f, %f, %f", fuck3.x, fuck3.y, fuck3.z);

	// auto fuck4 = tr::Vec4<float64>(1, 2, 3, 4) % tr::Vec4<float64>(5, 6, 7, 8);
	// tr::log("%f, %f, %f, %f", fuck4.x, fuck4.y, fuck4.z, fuck4.w);

	// tr::Maybe<int32> maybe;
	// if (!maybe.is_valid()) {
	// 	tr::log("nope");
	// }
	// // maybe.unwrap();
	// maybe = 57;
	// if (maybe.is_valid()) {
	// 	tr::log("perchance %i", maybe.unwrap());
	// }

	// auto it_depends_yknow = tr::Either<int32, bool>(64);
	// if (it_depends_yknow.is_left()) {
	// 	tr::log("left: %i", it_depends_yknow.left());
	// }
	// it_depends_yknow.right();

	// auto should_i_call_it_both = tr::Pair<int64, float64>(1, 2.2);
	// tr::log("pair left: %li, pair right %f", should_i_call_it_both.left, should_i_call_it_both.right);

	// tr::MaybeRefma<tr::Arena> arena = new tr::Arena(tr::kb_to_bytes(2));
	// // arena.prealloc(tr::mb_to_bytes(1));
	// struct ComicallyLargeStruct {
	// 	uint8 man[1024];
	// };
	// struct EvenLargerStruct {
	// 	uint8 man[4096];
	// };
	// // if it works it won't segfault
	// // auto* sig = arena->alloc<ComicallyLargeStruct>();
	// // auto* ma = arena->alloc<EvenLargerStruct>();
	// // tr::log("it did allocate");
	// // sig->man[12] = 221;
	// // ma->man[35] = 54;
	// // tr::log("look %i", sig->man[12]);
	// // tr::log("oh look at what you've done now to me %i", ma->man[35]);

	// auto random = tr::Random();
	// tr::log("sogmarand %f", random.next(1.0, 1.4));
	// tr::log("sogmarand %li", random.next<int64>(1, 69'000'000'000));

	// auto lerp_a = tr::Vec2<int64>(1, 2);
	// auto lerp_b = tr::Vec2<int64>(15393, 2376);
	// tr::Vec2<int64> lerp = tr::lerp(lerp_a, lerp_b, 0.5);
	// tr::log("lerp vec2<int64>: %li, %li", lerp.x, lerp.y);

	// tr::String str = "sigma";
	// tr::log("str: %s (length %zu)", str.buf(), str.len());

	// tr::String maballs = tr::sprintf(arena, 256, "%s balls", str.buf());
	// tr::log("%s", maballs.buf());

	// TR_ASSERT(str == "sigma");
	// TR_ASSERT(str != "ballshshjs");
	// TR_ASSERT(str != "sigmaaaa pelotas");
	// TR_ASSERT(str.substr(arena, 1, 3) == "igm");
	// // tr::Array<usize> sigma = tr::String("sigmysigmy").find(arena, "ig");
	// // TR_ASSERT(sigma.length() == 2);
	// // tr::String sigmaa = tr::String("figma").concat(arena, " balls");
	// // TR_ASSERT(sigmaa == "figma balls");
	// // TR_ASSERT(sigmaa.starts_with("figm"));
	// // TR_ASSERT(sigmaa.ends_with("alls"));

	// // TR_ASSERT(tr::String("/path/to/file.txt").file(arena) == "file.txt");
	// // TR_ASSERT(tr::String("/path/to/file.txt").directory(arena) == "/path/to");
	// // TR_ASSERT(tr::String("/path/to/teapot.blend.1").extension(arena) == ".blend.1");
	// // TR_ASSERT(tr::String("app://sigma").is_absolute());
	// // TR_ASSERT(tr::String("C:\\sigma").is_absolute());
	// // TR_ASSERT(!tr::String("sigma").is_absolute());
	// // TR_ASSERT(!tr::String("./sigma").is_absolute());
	// // TR_ASSERT(tr::String("~/sigma").is_absolute());

	// // man
	// // tr::MemoryInfo memory = tr::get_memory_info();
	// // couldn't be bothered to print shit so i just read it in gdb
	// // tr::panic("lol");

	// tr::Ref<tr::List<tr::String>> listma = new tr::List<tr::String>();
	// listma->add("fuck...");
	// listma->add("shit...");
	// listma->add("crap...");
	// listma->add("WRONG");
	// (*listma)[3] = "correct...";
	// for (auto s : *listma) {
	// 	tr::log("list[%zu] = %s", s.i, s.val.buf());
	// }

	// // man
	// tr::Ref<tr::HashMap<tr::String, tr::String>> hashma = new tr::HashMap<tr::String, tr::String>();
	// (*hashma)["Sigma"] = "balls!";
	// tr::log("hashma[\"Sigma\"] = \"%s\"", (*hashma)["Sigma"].buf());

	// // check collsiions
	// // what the fuck
	// using Disaster = tr::AdvancedHashMap<tr::String, tr::String, custom_hash_func, 256>;
	// tr::Ref<Disaster> hashmaballs = new Disaster();
	// (*hashmaballs)["Sigma"] = "balls!";
	// (*hashmaballs)["Balls"] = "sigma!";
	// (*hashmaballs)["oh no it go it gone bye bye (bye)"] = "ball. one";
	// (*hashmaballs)["oSgmggg"] = "ball. one ball.";
	// hashmaballs->remove("oh no it go it gone bye bye (bye)");
	// tr::log("hashmaballs[\"Sigma\"] = \"%s\"", (*hashmaballs)["Sigma"].buf());
	// tr::log("hashmaballs[\"Balls\"] = \"%s\"", (*hashmaballs)["Balls"].buf());
	// tr::log("hashmaballs[\"oSgmggg\"] = \"%s\"", (*hashmaballs)["oSgmggg"].buf());

	// // check resizing
	// // this will actually resize twice
	// tr::Ref<tr::HashMap<int32, bool>> whathteufc = new tr::HashMap<int32, bool>();
	// for (int32 i = 0; i < 257; i++) {
	// 	(*whathteufc)[i] = i % 2 == 0;
	// }
	// // i'm just gonna find that in the log file
	// // having log files is great
	// whathteufc->remove(69);

	// // iterator
	// for (auto item : *whathteufc) {
	// 	tr::log("number %i = %s", item.left, item.right ? "true" : "false");
	// }
	// tr::log("length %zu, capacity %zu", whathteufc->length(), whathteufc->capacity());

	// // split string :)
	// tr::String strma = "crap,shit,fuck,balls";
	// tr::Array<tr::String> splitma = strma.split(arena, ',');
	// for (auto c : splitma) {
	// 	tr::log("split[%zu] = %s", c.i, c.val.buf());
	// }

	// tr::Ref<tr::File> write_file = tr::File::open("sigma.txt", tr::FileMode::WRITE_TEXT).unwrap();
	// write_file->write_string("Fuck...\r\nYou...", false);
	// write_file->flush();
	// // close immediately pls
	// write_file->close();

	// tr::Ref<tr::File> read_file = tr::File::open("sigma.txt", tr::FileMode::READ_TEXT).unwrap();
	// tr::String line1 = read_file->read_line(arena);
	// tr::String line2 = read_file->read_line(arena);
	// tr::log("file: %li bytes\n%s\n%s", read_file->len(), line1.buf(), line2.buf());
	// read_file->close();

	// tr::rename_file("sigma.txt", "die.txt");
	// tr::remove_file("die.txt");

	// TR_ASSERT(tr::create_dir("sigmyyyyls"));

	// tr::free();
}
