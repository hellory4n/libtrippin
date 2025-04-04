#include <time.h>
#include "libtrippin.h"

int main(void) {
	tr_init("log.txt");

	for (size_t i = 0; i < 10; i++) {
		double sorandomxd = tr_rand_double(tr_default_rand(), 1, 10);
		tr_log(TR_LOG_INFO, "%f", sorandomxd);
	}

	// you can make some colors
	TrColor color = tr_hex_rgba(0x448AFFFF);
	tr_log(TR_LOG_INFO, "%.2X%.2X%.2X%.2X", color.r, color.g, color.b, color.a);

	tr_free();
}
