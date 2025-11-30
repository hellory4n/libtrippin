#include <cassert>

#include <trippin/buffer.h>
#include <trippin/macros.h>
#include <trippin/memory.h>

using namespace tr;

int main()
{
	// this is the same as test/memory.cpp but using Buffer<T> instead of raw ptrs
	constexpr usize LEN = sizeof("heheheha");
	Buffer<const char> other_buffer = {"heheheha"};
	Buffer<char> buffer = memnew_buffer<char>(LEN);

	memcopy(buffer, {"heheheha"});
	assert(memequal(buffer, other_buffer));

	memreset(buffer);
	char empty_buffer_type_shit[LEN] = {};
	assert(memequal(buffer, Buffer{empty_buffer_type_shit}));

	// making sure this compiles
	[[maybe_unused]]
	Buffer<const char> const_buffer = other_buffer;

	memfree(buffer);
	// double free should be ignored
	memfree(buffer);
}
