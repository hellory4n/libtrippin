# libtrippin

Most biggest most massive library of all time. I'm insane.

## Featuring

- [libtrippin](./libtrippin.h) v2.2.0: Most massive library of all time
    - C++17 with no external dependencies (only libc/stdc++)
    - Arenas and reference counting
    - Arrays, lists, hashmaps, strings
    - Basic math
    - Logging
    - Files/IO
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
tr::Array<int32> array(arena, TR_ARRLEN(int32, items));
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

### Lists and hashmaps

```cpp
#include <trippin/collection.hpp>

// lists can change size
tr::Ref<tr::List<int32>> list = new tr::List<int32>();
list->add(1);
list->add(2);
list->add(848068024);

for (auto item : *list) {
    tr::log("list[%zu] = %i", item.i, item.val);
}

// hashmaps are hashmaps lmao
// quite the fucking mouthful but it's just HashMap<String, String>
auto hashmap = tr::Ref<tr::HashMap<tr::String, tr::String>>(new tr::HashMap<tr::String, tr::String>());
(*hashmap)["john"] = "bob";
(*hashmap)["bob"] = "greg";
(*hashmap)["greg"] = "craig";
(*hashmap)["craig"] = "fuck craig";
hashmap->remove("craig");

// hashmaps aren't sorted
// so it'll show up in a seemingly random order
for (auto item : *hashmap) {
    tr::log("hashmap['%s'] = '%s'", item.left.buffer(), item.right.buffer());
}
```

### Files

```cpp
#include <trippin/iofs.hpp>

// reading
tr::Ref<tr::File> file = tr::File::open("file.txt", tr::FileMode::READ_TEXT).unwrap();
tr::String line = file->read_line(arena);
// it closes automatically!

// writing
tr::Ref<tr::File> file = tr::File::open("otherfile.bin", tr::FileMode::WRITE_BINARY).unwrap();
file->write_string("Man...\nso true", false);
file->write_struct(2758952);
```

## FAQ

### Have you tried \[language] you fucking moron

No fuck off.

### Why?

That's enough questions.

## Copyright pls don't sue me cheers mate

- `trippin/libxxhash.h`: Copyright (c) 2012-2021 Yann Collet, licensed under the BSD-2-Clause License
- `trippin/libbackward.hpp`: Copyright 2013 Google Inc., licensed under the MIT license
- Everything else: Copyright (C) 2025 by hellory4n, licensed under the BSD-0-Clause License
