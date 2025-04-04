#include <stdint.h>
#include "libtrippin.h"

int main(void) {
	tr_init("log.txt");

	// slices require an arena
	TrArena arena = tr_arena_new(TR_MB(1));

	TrSlice slicema = tr_slice_new(arena, 4, sizeof(int32_t));

	// set items
	*(int32_t*)tr_slice_at(slicema, 0) = 11;
	*(int32_t*)tr_slice_at(slicema, 1) = 22;
	*(int32_t*)tr_slice_at(slicema, 2) = 33;
	*(int32_t*)tr_slice_at(slicema, 3) = 44;

	// iterate
	for (size_t i = 0; i < slicema.length; i++) {
		tr_log(TR_LOG_INFO, "%i\n", *(int32_t*)tr_slice_at(slicema, i));
	}

	// will panic without mysteriously dying
	tr_log(TR_LOG_INFO, "%i\n", *(int32_t*)tr_slice_at(slicema, 8268286804));

	// you free the entire arena at once
	tr_arena_free(arena);

	tr_free();
}
