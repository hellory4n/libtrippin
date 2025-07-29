#include <trippin/collection.h>

int main(void)
{
	// hashmaps are hashmaps lmao
	tr::HashMap<tr::String, tr::String> map(tr::scratchpad());
	map["john"] = "bob";
	map["bob"] = "greg";
	map["greg"] = "craig";
	map["craig"] = "fuck craig";
	map.remove("craig");

	// hashmaps aren't sorted
	// so it'll show up in a seemingly random order
	for (auto [key, value] : map) {
		tr::log("hashmap['%s'] = '%s'", *key, *value);
	}

	// there's also signals so that's cool
	tr::Signal<int64> signa(tr::scratchpad());
	signa.connect([&](int64 x) -> void {
		tr::log("SOMETHING HAS HAPPENED???");
	});
	signa.emit(759823);
}
