#include <cassert>

#include <trippin/buffer.h>
#include <trippin/memory.h>

using namespace tr;

int main()
{
	Buffer<char> buffer = memnew_buffer<char>(sizeof("balls"));
	memfree(buffer);
	buffer[4] = '.';
}
