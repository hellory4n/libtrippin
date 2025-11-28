#include <cstdio>
#include <cstring>

#include <trippin/macros.h>
#include <trippin/memory.h>

using namespace tr;

int main()
{
	char* buffer = memnew<char>(sizeof("heheheha"));
	TR_DEFER(memfree(buffer));

	memcpy(buffer, "heheheha", sizeof("heheheha"));
	printf("%s\n", buffer);
}
