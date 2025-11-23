#pragma once
#include <trippin/error.h>
#include <trippin/memory.h>

#include "trippin/string.h"

// make your own error types
// you can read the documentation for each thing for more detail
tr::TempString errmsg_unexpected_happening(tr::ErrorArgs args)
{
	return tr::tmp_fmt("unexpected happening it is happening unexpectedly: %s", *args[0].str);
}
constexpr tr::ErrorType ERROR_UNEXPECTED_HAPPENING =
	tr::errtype_from_string("app::UNEXPECTED_HAPPENING");
TR_REGISTER_ERROR_TYPE(ERROR_UNEXPECTED_HAPPENING, errmsg_unexpected_happening);

tr::Result<int32> example_function()
{
	// on error
	return {ERROR_UNEXPECTED_HAPPENING, "this is unfortunate"};

	// on success you can just return as usual
	return 946259;
}

// usage
tr::Result<void> trippin_main()
{
	// .unwrap() is for if you're *really* sure that it won't fail
	// it's not recommended because it will panic on fail
	int32 x = example_function().unwrap();

	// usually you should use the TR_TRY macros instead
	int32 y = TR_TRY(example_function());
	// which is equivalent to
	auto tmp = example_function();
	if (!tmp.is_valid()) {
		return tmp.unwrap_err();
	}
	int32 z = tmp.unwrap();
	// but you can only use TR_TRY in functions that return tr::Result<T>

	// also, TR_TRY is only for variables
	// so this won't work: function(TR_TRY(other_function()));

	return {};
}

void test_error()
{
	trippin_main().unwrap();
}
