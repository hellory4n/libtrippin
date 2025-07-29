#include <trippin/common.h>
#include <trippin/log.h>
#include <trippin/string.h>
#include <trippin/math.h>
#include <trippin/error.h>

int main(void)
{
	// maybe
	// for references/pointers use tr::MaybePtr<T>
	tr::Maybe<int32> nope;
	if (nope.is_valid()) {
		tr::log("%i", nope.unwrap());
	}

	// either
	tr::Either<int32, tr::String> depends(5);
	if (depends.is_left()) {
		tr::log("%i", depends.left());
	}

	// pair
	tr::Pair<int32, tr::String> both(27652465, "the of when of who why how");
	tr::log("%i", both.left);
	tr::log("%s", *both.right);

	// colors
	constexpr tr::Color fresh_brown_poo = tr::Color::rgb(0x734a16);
}
