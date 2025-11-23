#pragma once
#include <trippin/common.h>
#include <trippin/log.h>
#include <trippin/string.h>

[[noreturn /* shut up */]]
void test_logging()
{
	tr::use_log_file("log.txt");
	tr::init();

	tr::log("sir");
	tr::info("sir");
	tr::warn("sir");
	tr::error("sir");

	TR_ASSERT(true != false);
	TR_ASSERT_MSG(2 + 2 == 5, "i might be wrong");
	tr::panic("AHHHHHHH");

	tr::free();
}
