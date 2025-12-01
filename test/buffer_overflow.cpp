#include <cassert>

#include <trippin/buffer.h>
#include <trippin/macros.h>
#include <trippin/memory.h>

using namespace tr;

int main()
{
	Buffer<char> buffer = memnew_buffer<char>(sizeof("balls"));
	TR_DEFER(memfree(buffer));
	// balls...
	buffer[5] = '.';
	buffer[6] = '.';
	buffer[7] = '.';
}
