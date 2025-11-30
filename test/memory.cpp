#include <cassert>

#include <trippin/macros.h>
#include <trippin/memory.h>

using namespace tr;

int main()
{
	constexpr usize LEN = sizeof("heheheha");
	char* buffer = memnew<char>(LEN);

	memcopy(buffer, "heheheha", LEN);
	assert(memequal(buffer, LEN, "heheheha", LEN));

	memreset(buffer, LEN);
	char empty_buffer_type_shit[LEN] = {};
	assert(memequal(buffer, LEN, empty_buffer_type_shit, LEN));

	memfree(buffer);
	// double free should be ignored
	memfree(buffer);
}
