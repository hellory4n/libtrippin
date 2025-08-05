#include <trippin/log.h>
#include <trippin/math.h>

int main()
{
	tr::Random sorandomxd;
	tr::Vec3<float32> vecma;
	for (auto i : tr::range<usize>(0, 3)) {
		vecma[i] = sorandomxd.next(0.0f, 999999999.9f);
	}
	TR_ASSERT(vecma.x > 0.0f);
}
