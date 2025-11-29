#include <cassert>

#include <trippin/buffer.h>
#include <trippin/macros.h>
#include <trippin/memory.h>

using namespace tr;

int main()
{
	// this is the same as test/memory.cpp but using Buffer<T> instead of raw ptrs
	constexpr usize LEN = sizeof("heheheha");
	Buffer<char> buffer{memnew<char>(LEN), LEN};

	memcopy(buffer, {"heheheha", LEN});
	assert(memequal(buffer, {"heheheha", LEN}));

	memreset(buffer, LEN);
	char* empty_buffer_type_shit[LEN] = {};
	assert(memequal(buffer, LEN, empty_buffer_type_shit, LEN));

	memfree(buffer);
	// double free should be ignored
	memfree(buffer);
}
