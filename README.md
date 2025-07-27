# libtrippin

Most biggest most massive library of all time. I'm insane.

## Featuring

- [libtrippin](./libtrippin.h) v2.4.0: Most massive library of all time
    - C++17 with no external dependencies (only libc/stdc++)
    - Cross-platform (Windows and POSIX)
    - Arenas for memory management
    - Arrays, strings, hashmaps
    - Simple math for games
    - Logging
    - Files/IO
    - Functional errors
    - And more

## Usage

Make sure you're using C++17, it won't compile with anything older.

Now add all the `.hpp`/`.cpp` files from `trippin/` to your project

It should compile with GCC, Clang, and MSVC (Visual Studio)

On Linux you also have to link with the math library `-lm`

Also make sure to add this at the start of your program:

```cpp
tr::use_log_file("log.txt");
tr::init();
```

And at the end of your program:

```cpp
tr::free();
```

## Examples

See the docs folder for more crap.

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

TR_ASSERT_MSG(2 + 2 == 5, "i might be wrong");
tr::panic("AAAAAHHH");

tr::free();
```

### Memory

```cpp
#include <trippin/log.hpp>
#include <trippin/memory.hpp>

tr::Arena arena;

// arenas are infinite
// allocate as much stuff as you want!
auto* crap = static_cast<CrapStruct*>(arena.alloc(sizeof(CrapStruct)));
// nicer wrapper
// it even supports passing arguments to the constructor
CrapStruct& crap = arena.make<CrapStruct>();

// you can also allocate arrays
// tr::scratchpad() is a temporary arena (like a sane alloca)
tr::Array<int64> array(tr::scratchpad(), {1, 2, 3, 4, 5});
array.add(6);
for (auto [i, num] : array) {
    tr::log("array[%zu] = %li", i, num);
}
```

### Strings

```cpp
#include <trippin/string.hpp>

// temporary string
tr::String str = "hi mom";
tr::log("%s", str.buf());

// arena string
tr::String str(arena, "a string", sizeof("a string"));

// you can do formatting too
tr::String str = tr::fmt(arena, "hi %s", "mom");
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

### Collections

```cpp
#include <trippin/collection.hpp>

// hashmaps are hashmaps lmao
tr::HashMap<tr::String, tr::String> map(arena);
map["john"] = "bob";
map["bob"] = "greg";
map["greg"] = "craig";
map["craig"] = "fuck craig";
map.remove("craig");

// hashmaps aren't sorted
// so it'll show up in a seemingly random order
for (auto [key, value] : map) {
    tr::log("hashmap['%s'] = '%s'", key.buf(), value.buf());
}

// there's also signals so that's cool
tr::Signal<int64> signa(arena);
signa.connect([&](int64 x) -> void {
    tr::log("SOMETHING HAS HAPPENED???");
});
signa.emit(759823);
```

### Files

```cpp
#include <trippin/iofs.hpp>

// reading
// .unwrap() will panic on error, you probably shouldn't do that in most cases
tr::File& file = *tr::File::open(arena, "file.txt", tr::FileMode::READ_TEXT).unwrap();
tr::String line = file.read_line(arena).unwrap();
// it closes automatically!

// writing
tr::File& file = *tr::File::open(arena, "otherfile.bin", tr::FileMode::WRITE_BINARY).unwrap();
file->write_string("Man...\nso true");
file->write_struct(2758952);
```

## FAQ

### Have you tried \[language] you fucking moron

No fuck off.

### Why?

That's enough questions.
