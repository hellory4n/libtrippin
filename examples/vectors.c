#include "libtrippin.h"
#include <stdio.h>

int main(void)
{
	TrippinVec2f vec2 = {1, 2};
	TrippinVec2f result1 = tv2_add(vec2, vec2);
	printf("%f, %f\n", result1.x, result1.y);

	// you can mix and match types
	// but not vector2s and 3s
	TrippinVec2f vec2f = {1.5, 2.5};
	TrippinVec2i vec2i = {2, 2};
	TrippinVec2f result2 = tv2_sub(vec2f, vec2i);
	printf("%f, %f\n", result2.x, result2.y);
	return 0;
}
