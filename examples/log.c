#include "libtrippin.h"

int main(void) {
	// a context is required for logging to work
	TrippinContext* ctx = trippin_init("log.txt");

	trippin_log(ctx, TRIPPIN_LOG_LIB_INFO, "im having a stroke");
	trippin_log(ctx, TRIPPIN_LOG_INFO, "im having a stroke (not libtrippin)");
	trippin_log(ctx, TRIPPIN_LOG_WARNING, "we will all die.");
	trippin_log(ctx, TRIPPIN_LOG_ERROR, "im died");

	trippin_assert(ctx, false, "seems %s", "bad");
	trippin_panic(ctx, "AAAAAAAHHHH");

	trippin_free(ctx);
}
