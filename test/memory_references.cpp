#include <cassert>

#include <trippin/macros.h>
#include <trippin/memory.h>

using namespace tr;

int main()
{
	// references have to reference something
	int a = 1, b = 2, c = 3, d = 4;

	RefWrapper<int&>* buffer = memnew<RefWrapper<int&>>(sizeof(int) * 4);
	TR_DEFER(memfree(buffer));

	// compiler pls dont optimize this away thanks
	buffer[0] = a;
	buffer[1] = b;
	buffer[2] = c;
	buffer[3] = d;
	assert(buffer[0] == 1);
	assert(buffer[1] == 2);
	assert(buffer[2] == 3);
	assert(buffer[3] == 4);
}
