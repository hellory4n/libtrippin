#include <stdint.h>
#include <stdio.h>
#include "libtrippin.h"

int main(void) {
	TrippinSlice slicema = trippin_slice_new(4, sizeof(int32_t));

	// set items
	*(int32_t*)trippin_slice_at(slicema, 0) = 11;
	*(int32_t*)trippin_slice_at(slicema, 1) = 22;
	*(int32_t*)trippin_slice_at(slicema, 2) = 33;
	*(int32_t*)trippin_slice_at(slicema, 3) = 44;

	// iterate
	for (size_t i = 0; i < slicema.length; i++) {
		printf("%i\n", *(int32_t*)trippin_slice_at(slicema, i));
	}

	// will panic without mysteriously dying
	printf("%i\n", *(int32_t*)trippin_slice_at(slicema, 8268286804));

	trippin_slice_free(slicema);
}
