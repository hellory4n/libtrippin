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
		tr_log(TR_LOG_INFO, "%i", *(int32_t*)tr_slice_at(slicema, i));
	}
	// will panic without mysteriously dying
	tr_log(TR_LOG_INFO, "%i\n", *(int32_t*)tr_slice_at(slicema, 8268286804));

	// 2d slice
	TrSlice2D slicema2d = tr_slice2d_new(arena, 4, 4, sizeof(int32_t));

	// set items
	*(int32_t*)tr_slice2d_at(slicema2d, 0, 0) = 11;
	*(int32_t*)tr_slice2d_at(slicema2d, 1, 1) = 22;
	*(int32_t*)tr_slice2d_at(slicema2d, 2, 2) = 33;
	*(int32_t*)tr_slice2d_at(slicema2d, 3, 3) = 44;

	// iterate
	for (size_t y = 0; y < slicema2d.height; y++) {
		for (size_t x = 0; x < slicema2d.width; x++) {
			tr_log(TR_LOG_INFO, "%i", *(int32_t*)tr_slice2d_at(slicema2d, x, y));
		}
	}
	// will panic without mysteriously dying
	tr_log(TR_LOG_INFO, "%i\n", *(int32_t*)tr_slice2d_at(slicema2d, 8268286804, 6247235784864));

	// you free the entire arena at once
	tr_arena_free(arena);

	tr_free();
}
