#pragma once
#include <trippin/common.h>
#include <trippin/log.h>

// just so clang shuts up :)
[[noreturn]]
void test_logging()
{
	tr::use_log_file("log.txt");
	tr::init();

	tr::log("sir");
	tr::info("sir");
	tr::warn("sir");
	tr::error("sir");

	TR_ASSERT_MSG(2 + 2 == 5, "i might be wrong");
	tr::panic("AHHHHHHH");

	tr::free();
}
