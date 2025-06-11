#include "libtrippin.hpp"

int main(void)
{
	tr::Random sorandomxd;
	tr::Vec3<float32> vecma;
	for (usize i = 0; i < 3; i++) {
		vecma[i] = sorandomxd.next(0.f, 999999999.f);
	}
	tr::assert(vecma.x > 0.f, "oh no");
}
