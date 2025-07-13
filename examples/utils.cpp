#include <trippin/common.hpp>
#include <trippin/log.hpp>
#include <trippin/string.hpp>
#include <trippin/math.hpp>
#include <trippin/error.hpp>

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
	tr::log("%s", both.right.buf());

	// colors
	constexpr tr::Color fresh_brown_poo = tr::Color::rgb(0x734a16);

	// error handling
	// the error type must implement tr::Error
	tr::Result<int64, tr::Error> itscomplicated = tr::StringError("oh crap");

	// check if it's valid
	// if .unwrap() fails it'll print the error message and panic
	if (itscomplicated.is_valid()) {
		tr::log("%li", itscomplicated.unwrap());
	}
}
