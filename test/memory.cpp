#include <cassert>

#include <trippin/macros.h>
#include <trippin/memory.h>

using namespace tr;

int main()
{
	char* buffer = memnew<char>(sizeof("heheheha"));
	TR_DEFER(memfree(buffer));

	memcopy(buffer, "heheheha", sizeof("heheheha"));
	assert(memequal(buffer, sizeof("heheheha"), "heheheha", sizeof("heheheha")));
}
