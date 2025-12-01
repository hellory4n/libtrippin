#include <cassert>

#include <trippin/array.h>
#include <trippin/buffer.h>

using namespace tr;

int main()
{
	Array<5, int> la_comedia = {69, 67, 61, 41, 38};
	int how_memory_safe[5] = {69, 67, 61, 41, 38};
	assert(memequal(as<Buffer<int>>(la_comedia), Buffer{how_memory_safe}));
	la_comedia[1] = -67;
	assert(!memequal(as<Buffer<int>>(la_comedia), Buffer{how_memory_safe}));
}
