# libtrippin v0.1.0

Some utilities craps because the C standard library is tiny

## Features

i'll remove the checklist when it's done

- Reference counting through a compiler hack (supported by gcc and clang)
- Math functions
- Basic vectors (the math kind)
- Logging, assert, panic
- Slices (can't change size), 2d slices, and strings
- Sexually attractive macros that makes C sexually attractive
- Not lists or hashmaps, use [stb_ds](https://github.com/nothings/stb/blob/master/stb_ds.h)
- And more

### todo

- [x] reference counting
- [x] vectors
- [x] logging
- [ ] slices
- [ ] strings
- [ ] 2d slices
- [ ] more math
- [ ] more documentation (some examples in the readme, conventions.md)

### Exploiting macros

Libtrippin provides several handsome macros that make C look like a modern language\*\*\* terms and conditions
apply

You can disable the macros by defining `TRIPPIN_NO_SHORTHAND` before including `libtrippin.h`, and you
won't lose any functionality by doing so, only sex appeal (it's still available either through functions or longer macros)