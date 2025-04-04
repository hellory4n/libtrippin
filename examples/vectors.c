#include "libtrippin.h"

int main(void)
{
	tr_init("log.txt");

	TrVec2f vec2 = {1, 2};
	TrVec2f result1 = TR_V2_ADD(vec2, vec2);
	tr_log(TR_LOG_INFO, "%f, %f\n", result1.x, result1.y);

	// you can mix and match types
	// but not vector2s and 3s
	TrVec2f vec2f = {1.5, 2.5};
	TrVec2i vec2i = {2, 2};
	TrVec2f result2 = TR_V2_SUB(vec2f, vec2i);
	tr_log(TR_LOG_INFO, "%f, %f\n", result2.x, result2.y);

	tr_free();
	return 0;
}
