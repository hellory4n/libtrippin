#include <time.h>
#include "libtrippin.h"

int main(void) {
	tr_init("log.txt");

	for (size_t i = 0; i < 10; i++) {
		double sorandomxd = tr_rand_f64(tr_default_rand(), 1, 10);
		tr_log(TR_LOG_INFO, "%f", sorandomxd);
	}

	tr_free();
}
