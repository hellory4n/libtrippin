#include "libtrippin.hpp"

int main(void)
{
	// maybe
	// for pointers use tr::MaybeRef<T>, see memory.cpp
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
	tr::log("%s", both.right.buffer());

	// colors
	constexpr tr::Color fresh_brown_poo = tr::Color::rgb(0x734a16);
}
