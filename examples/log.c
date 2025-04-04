#include "libtrippin.h"

int main(void) {
	tr_init("log.txt");

	tr_log(TR_LOG_LIB_INFO, "im having a stroke");
	tr_log(TR_LOG_INFO, "im having a stroke (not libtr)");
	tr_log(TR_LOG_WARNING, "we will all die.");
	tr_log(TR_LOG_ERROR, "im died");

	tr_assert(false, "seems %s", "bad");
	tr_panic("AAAAAAAHHHH");

	tr_free();
}
