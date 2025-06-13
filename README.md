# libtrippin

Most biggest most massive library of all time. I'm insane.

## Featuring

- [libtrippin](./libtrippin.h) v2.1.0: Most massive standard library
    - C++14 with no external dependencies (only libc)
    - Arenas
    - Reference counting
    - Arrays
    - Basic strings
    - Math
    - Logging
    - And more
- [engineer](./engineerbuild/README.md) v1.1.0: Build system of all time
    - No esoteric language
    - Only external dependency is lua
    - You can specify options and stuff
    - You can control the entire build process
    - Incremental builds
    - Support for clangd's `compile_commands.json`

## Usage

Just add `libtrippin.cpp` to your project

If you have debug builds make sure you have `DEBUG` defined (it doesn't change a whole lot but i mean why not)

On Linux make sure you linked the math library `-lm`

If some for reason you want to use engineer then [read this](./engineerbuild/README.md)

## Examples

See the examples folder for more crap.

### Logging

```cpp
tr::use_log_file("log.txt");
tr::init();

// these are just for different colors
tr::log("sir");
tr::info("sir");
tr::warn("sir");
tr::error("sir");

// same formatting as printf
tr::info("sir %i", 9462952);

tr::assert(2 + 2 == 5, "i may be wrong");
tr::panic("AAAAAHHH");

tr::free();
```

### Memory

```cpp
// initialize an arena
// tr::Ref is for reference counting
tr::Ref<tr::Arena> arena = new tr::Arena(tr::kb_to_bytes(64));

// arenas are infinite
// allocate as many as you want!
auto* crap = arena->alloc<CrapStruct>();

// you can also allocate arrays
int32 items[] = {1, 2, 3, 4, 5};
tr::Array<int32> array(arena, items, 5);
array.add(6);
for (auto item : array) {
    tr::log("array[%zu] = %i", item.i, item.val);
}
```

### Strings

```cpp
// temporary string
tr::String str = "hi mom";
tr::log("%s", str.buffer());

// arena string
tr::String str(arena, "a string", sizeof("a string"));

// you can do formatting too
tr::String str = tr::sprintf(arena, 32, "hi %s", "mom");
```

### Math

```cpp
tr::Random sorandomxd;
tr::Vec3<float32> vecma;
for (usize i = 0; i < 3; i++) {
    vecma[i] = sorandomxd.next(0.f, 999999999.f);
}
tr::assert(vecma.x > 0.f, "oh no");
```

## FAQ

### Have you tried \[language] you fucking moron

No fuck off.

### Why?

That's enough questions.
