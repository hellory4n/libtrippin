# libtrippin v1.2.1

Most biggest most massive standard library thing for C of all time

## Features

- Arenas
- Math functions, custom random functions (xoshiro256+ (i probably won't remember what algorithm i used))
- Basic vectors and colors (the math kind)
- Logging, assert, panic
- Not lists or hashmaps, use [stb_ds](https://nothings.org/stb_ds)
- Slices and 2d slices (slices are arrays but stb calls their lists arrays so that'd be weird, btw stb arrays are more like a stack/queue, did you even read that link? our relationship isn't working out.)
- The whole thing is 2 files with no external dependencies, only C99

## Usage

Put libtrippin.h and libtrippin.c somewhere in your project.

Tell your build system to compile libtrippin.c.

If you have debug builds make sure you have `DEBUG` defined (it doesn't change a whole lot but i mean why not)

On Linux make sure you linked the math library `-lm`

## Example

Look at the examples folder for something comprehensible

```c
#include "libtrippin.h"

int main(void)
{
    tr_init("log.txt");
    TrArena arena = tr_arena_new(TR_MB(1));
    TrSlice_Vec2f slicema = tr_slice_new(arena, 4, sizeof(TrVec2f));
    TrRand rand = tr_rand_new(123456789);

    TrVec2f vecdeez = {tr_rand_double(&rand, 1, 10), tr_rand_double(&rand, 1, 10)};
    TR_SET_SLICE(&arena, &slicema, TrVec2f,
        (TrVec2f){1, 2},
        vecdeez,
        TR_V2_ADD(vecdeez, vecdeez),
        TR_V2_SMUL(vecdeez, 2),
    );

    for (size_t i = 0; i < slicema.length; i++) {
        TrVec2f vecm = *TR_AT(slicema, TrVec2f, i);
        tr_log( "%f, %f", vecm.x, vecm.y);
    }

    tr_arena_free(arena);
    tr_free();
}
```

## FAQ

### Have you tried \[language] you fucking moron

No fuck off.

### Why?

That's enough questions.
