#include "libtrippin.h"

int main(void) {
	trippin_init("log.txt");

	trippin_log(TRIPPIN_LOG_LIB_INFO, "im having a stroke");
	trippin_log(TRIPPIN_LOG_INFO, "im having a stroke (not libtrippin)");
	trippin_log(TRIPPIN_LOG_WARNING, "we will all die.");
	trippin_log(TRIPPIN_LOG_ERROR, "im died");

	trippin_assert(false, "seems %s", "bad");
	trippin_panic("AAAAAAAHHHH");

	trippin_free();
}
