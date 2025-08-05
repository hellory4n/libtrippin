#include <trippin/error.h>
#include <trippin/memory.h>

tr::Result<int32, tr::Error&> example_function()
{
	// on error
	// you can use any type that implements tr::Error
	return tr::scratchpad().make<tr::StringError>(
		"unexpected happening it is happening unexpectedly");

	// on success you can just return as usual
	return 946259;
}

// usage
tr::Result<void, tr::Error&> trippin_main()
{
	// .unwrap() is for if you're *really* sure that it won't fail
	// it's not recommended because it will panic on fail
	int32 x = example_function().unwrap();

	// usually you should use the TR_TRY* macros instead
	TR_TRY_ASSIGN(int32 x, example_function());
	// which expands to (roughly)
	auto tmp = example_function();
	if (!tmp.is_valid()) {
		return tmp.unwrap_err();
	}
	int32 x = tmp.unwrap();
	// but you can only use those macros in functions that return tr::Result<T, E>

	// additional macros:
	// TR_TRY is like TR_TRY_ASSIGN but ignoring the result
	TR_TRY(example_function());

	// TR_TRY_ASSERT returns an error instead of panicking on fail
	TR_TRY_ASSERT(2 + 2 == 5, tr::scratchpad().make<tr::StringError>("i might be wrong"));
}

int main()
{
	trippin_main().unwrap();
}
