#include <trippin/log.hpp>

int main(void)
{
	tr::use_log_file("log.txt");
	tr::init();

	tr::log("sir");
	tr::info("sir");
	tr::warn("sir");
	tr::error("sir");

	TR_ASSERT_MSG(2 + 2 == 5, "i may be wrong");
	tr::panic("AHHHHHHH");

	tr::free();
}
