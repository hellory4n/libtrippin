# libtrippin

Most biggest most massive library of all time. I'm insane.

## Featuring

- [libtrippin](./libtrippin.h) v2.1.2: Most massive standard library
    - C++14 with no external dependencies (only libc/stdc++)
    - Arenas
    - Reference counting
    - Arrays
    - Basic strings
    - Math
    - Logging
    - And more
- [engineer](./engineerbuild/README.md) v1.2.0: Build system of all time
    - No esoteric language
    - Only external dependency is lua
    - You can specify options and stuff
    - You can control the entire build process
    - Incremental builds
    - Support for clangd's `compile_commands.json`

## Usage

Just add all the `.hpp`/`.cpp` files from `trippin/` to your project

On Linux make sure you linked the math library `-lm`

libtrippin also uses [backward-cpp](https://github.com/bombela/backward-cpp) for better stack traces, it'll
compile by default but it's recommended you do extra setup on Linux:
- install binutils dev packages (e.g. `sudo apt install binutils-dev`)
- link with `bfd` and `dl`
- define `BACKWARD_HAS_BFD=1` (if you have libtrippin as a different project/static library, it has to be defined there)
It should work automatically on Windows.

If some for reason you want to use engineer then [read this](./engineerbuild/README.md)

## Examples

See the examples folder for more crap.

### Logging

```cpp
#include <trippin/log.hpp>

tr::use_log_file("log.txt");
tr::init();

// these are just for different colors
tr::log("sir");
tr::info("sir");
tr::warn("sir");
tr::error("sir");

// same formatting as printf
tr::info("sir %i", 9462952);

TR_ASSERT_MSG(2 + 2 == 5, "i may be wrong");
tr::panic("AAAAAHHH");

tr::free();
```

### Memory

```cpp
#include <trippin/log.hpp>
#include <trippin/memory.hpp>

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
#include <trippin/string.hpp>

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
#include <trippin/log.hpp>
#include <trippin/math.hpp>

tr::Random sorandomxd;
tr::Vec3<float32> vecma;
for (usize i = 0; i < 3; i++) {
    vecma[i] = sorandomxd.next(0.0f, 999999999.9f);
}
TR_ASSERT(vecma.x > 0.0f);
```

## FAQ

### Have you tried \[language] you fucking moron

No fuck off.

### Why?

That's enough questions.
