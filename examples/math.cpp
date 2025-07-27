#include <trippin/log.h>
#include <trippin/math.h>

int main(void)
{
	tr::Random sorandomxd;
	tr::Vec3<float32> vecma;
	for (usize i = 0; i < 3; i++) {
		vecma[i] = sorandomxd.next(0.0f, 999999999.9f);
	}
	TR_ASSERT(vecma.x > 0.0f);
}
