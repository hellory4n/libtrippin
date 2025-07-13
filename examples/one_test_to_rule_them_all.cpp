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

	tr::Array<int64> array(tr::scratchpad(), {1, 2, 3, 4, 5});
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

	tr::String maballs = tr::fmt(tr::scratchpad(), "%s balls", str.buf());
	TR_ASSERT(maballs == "sigma balls");
	tr::log("%s", maballs.buf());

	TR_ASSERT(str == "sigma");
	TR_ASSERT(str != "ballshshjs");
	TR_ASSERT(str != "sigmaaaa pelotas");
	TR_ASSERT(str.substr(tr::scratchpad(), 1, 3) == "igm");
	tr::Array<usize> sigma = tr::String("sigmysigmy").find(tr::scratchpad(), "ig");
	TR_ASSERT(sigma.len() == 2);
	tr::String sigmaa = tr::String("figma").concat(tr::scratchpad(), " balls");
	TR_ASSERT(sigmaa == "figma balls");
	TR_ASSERT(sigmaa.starts_with("figm"));
	TR_ASSERT(sigmaa.ends_with("alls"));

	TR_ASSERT(tr::String("/path/to/file.txt").file(tr::scratchpad()) == "file.txt");
	TR_ASSERT(tr::String("/path/to/file.txt").directory(tr::scratchpad()) == "/path/to");
	TR_ASSERT(tr::String("/path/to/teapot.blend.1").extension(tr::scratchpad()) == ".blend.1");
	TR_ASSERT(tr::String("app://sigma").is_absolute());
	TR_ASSERT(tr::String("C:\\sigma").is_absolute());
	TR_ASSERT(!tr::String("sigma").is_absolute());
	TR_ASSERT(!tr::String("./sigma").is_absolute());
	TR_ASSERT(tr::String("~/sigma").is_absolute());

	// split string :)
	tr::String strma = "crap,shit,fuck,balls";
	tr::Array<tr::String> splitma = strma.split(tr::scratchpad(), ',');
	for (auto c : splitma) {
		tr::log("split[%zu] = %s", c.i, c.val.buf());
	}
}

static void test::hashmaps()
{
	tr::log("\n==== HASHMAPS ====");

	tr::HashMap<tr::String, tr::String> hashma(tr::scratchpad());
	hashma["Sigma"] = "balls!";
	tr::log("hashma[\"Sigma\"] = \"%s\"", hashma["Sigma"].buf());

	// check collsiions
	tr::HashMapSettings<tr::String> settings{};
	settings.load_factor = 0.1;
	settings.initial_capacity = 4;
	settings.hash_func = [](const tr::String&) -> uint64 {
		return 68;
	};

	tr::HashMap<tr::String, tr::String> hashmaballs(tr::scratchpad(), settings);
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
	tr::File& wf = *tr::File::open(tr::scratchpad(), "fucker.txt", tr::FileMode::WRITE_TEXT).unwrap();
	wf.write_string("Crap crappington.\nother line");
	wf.close();

	tr::File& rf = *tr::File::open(tr::scratchpad(), "fucker.txt", tr::FileMode::READ_TEXT).unwrap();
	tr::String line1 = rf.read_line(tr::scratchpad()).unwrap();
	tr::String line2 = rf.read_line(tr::scratchpad()).unwrap();
	tr::log("line 1: %s; line 2: %s", line1.buf(), line2.buf());
	rf.close();

	tr::std_out.write_string("EVIL PRINTF FROM LIBTRIPPIN\n").unwrap();
	tr::std_out.write_string("please input some fucking bullshit: ").unwrap();
	tr::String line = tr::std_in.read_line(tr::scratchpad()).unwrap();
	tr::std_out.write_string("the fucking bullshit: ").unwrap();
	tr::std_out.write_string(line).unwrap();
	tr::std_out.write_string("\n").unwrap();

	// help.
	tr::move_file("fucker.txt", "fuckoffman.txt").unwrap();
	TR_ASSERT(tr::file_exists("fuckoffman.txt"));
	tr::remove_file("fuckoffman.txt").unwrap();

	tr::create_dir("crap/dir").unwrap();
	tr::remove_dir("dir").unwrap();
	tr::remove_dir("crap").unwrap();

	tr::Array<tr::String> crap = tr::list_dir(tr::scratchpad(), ".", false).unwrap();
	tr::log("this directory has: (not including hidden)");
	for (auto [_, name] : crap) {
		tr::log("- %s", name.buf());
	}

	TR_ASSERT(tr::is_file("log.txt").unwrap());
	TR_ASSERT(!tr::is_file("../").unwrap());
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

	// TODO this sucks
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
}
