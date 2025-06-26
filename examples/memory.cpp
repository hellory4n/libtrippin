#include <trippin/log.hpp>
#include <trippin/memory.hpp>

struct CrapStruct
{
	uint8 man[128];
};

int main(void)
{
	// initialize an arena
	// tr::Ref is for reference counting
	tr::Ref<tr::Arena> arena = new tr::Arena(tr::kb_to_bytes(64));

	// arenas are infinite
	// allocate as many as you want!
	// note this doesn't call the constructor, classes are instead supposed to take an arena in their
	// constructor
	auto* crap = arena->alloc<CrapStruct>();

	// you can also allocate arrays
	int32 items[] = {1, 2, 3, 4, 5};
	tr::Array<int32> array(arena, items, sizeof(items) / sizeof(int32));
	array.add(6);
	for (auto item : array) {
		tr::log("array[%zu] = %i", item.i, item.val);
	}

	// how to use reference counting
	// step 1: inherit tr::RefCounted
	class CrapClass : public tr::RefCounted
	{
	public:
		int32 some_var = 0;

		CrapClass()  { tr::log("hi");  }
		~CrapClass() { tr::log("bye"); }
	};

	// step 2: put it in tr::Ref<T>
	// now you should be able to just pass it around like any other pointer
	tr::Ref<CrapClass> classma = new CrapClass();
	classma->some_var = 563563;

	// tr::Ref<T> doesn't allow null
	// but if you want null, use tr::MaybeRef<T>
	tr::MaybeRef<CrapClass> perchance = nullptr;
}
