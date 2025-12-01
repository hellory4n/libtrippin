#include <cassert>

#include <trippin/math.h>

using namespace tr;

int main()
{
	// min/max do some template fuckery
	int a = min(1, -4, 6, 16, 61, -523, -3);
	assert(a == -523);

	int b = max(1, -4, 6, 16, 61, -523, -3);
	assert(b == 61);
}
