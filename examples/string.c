#include "libtrippin.h"

int main(void)
{
	trippin_init("log.txt");
	TrippinArena arenama = trippin_arena_new(TRIPPIN_MB(1));

	TrippinStr strma = tstr(arenama, "Hi mom");
	// print
	trippin_log(TRIPPIN_LOG_INFO, "%.*s", strma.length, strma.buffer);

	trippin_arena_free(arenama);
	trippin_free();
}
