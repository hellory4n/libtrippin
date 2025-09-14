# libtrippin

Most biggest most massive library of all time. I'm insane.

## Featuring

- [libtrippin](./trippin): Most massive library of all time
        - C++17 with no external dependencies (only libc/stdc++)
        - Cross-platform (Windows and POSIX)
        - Arenas for memory management
        - Arrays, strings, hashmaps
        - Simple math for games
        - Logging
        - Files/IO
        - Functional errors
        - And more
- [samurai](./samurai): Barely a build system
        - Literally just a [ninja](https://ninja-build.org/) script generator for Python
        - 200-ish lines of code you could easily write yourself

## Usage

Make sure you're using C++17, it won't compile with anything older.

Now add all the `.h`/`.cpp` files from `trippin/` to your project

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
#include <trippin/log.h>

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
#include <trippin/log.h>
#include <trippin/memory.h>

tr::Arena arena;
TR_DEFER(arena.free());

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
#include <trippin/string.h>

// temporary string
tr::String str = "hi mom";
tr::log("%s", *str);

// arena string
tr::String str(arena, "a string", sizeof("a string"));

// you can do formatting too
tr::String str = tr::fmt(arena, "hi %s", "mom");
```

### Math

```cpp
#include <trippin/log.h>
#include <trippin/math.h>

tr::Random sorandomxd;
tr::Vec3<float32> vecma;
for (auto i : tr::range<usize>(0, 3)) {
        vecma[i] = sorandomxd.next(0.0f, 999999999.9f);
}
TR_ASSERT(vecma.x > 0.0f);
```

### Collections

```cpp
#include <trippin/collection.h>

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
signa.connect([&](int64 _) -> void {
        tr::log("SOMETHING HAS HAPPENED???");
});
signa.emit(759823);
```

### Errors

```cpp
#include <trippin/error.h>
#include <trippin/memory.h>

tr::Result<int32, tr::Error&> example_function()
{
        // on error
        // you can use any type that implements tr::Error
        return tr::scratchpad().make<tr::StringError>("unexpected happening unexpectedly");

        // on success you can just return as usual
        return 946259;
}

// usage
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
```

### Files

```cpp
#include <trippin/iofs.h>

// reading
tr::File& file = tr::File::open(arena, "file.txt", tr::FileMode::READ_TEXT).unwrap();
TR_DEFER(file.close());
tr::String line = file.read_line(arena).unwrap();

// writing
tr::File& file = tr::File::open(arena, "otherfile.bin", tr::FileMode::WRITE_BINARY).unwrap();
TR_DEFER(file.close());
file.write_string("Man...\nso true");
file.write_struct(2758952);
```

## FAQ

### Have you tried \[language] you fucking moron

No fuck off.

### Why?

That's enough questions.
