# libtrippin

Files:
- `collection.*pp`: Additional collection types
- `common.*pp`: Numbers, macros, and utility structs
- `iofs.*pp`: Stream and filesystem APIs
- `log.*pp`: Logging, assert, and panic
- `math.*pp`: Vectors, matrices, RNG, and other math-y functions.
- `memory.*pp`: Reference counting, arenas, arrays, and a few utilities
- `string.*pp`: Strings :)

External libraries:
- [`libbackward.hpp`](https://github.com/bombela/backward-cpp): Prints stack traces when crashing
- [`libxxhash.h`](https://github.com/Cyan4973/xxHash): Hashing functions, used in `tr::hash` and `tr::HashMap<K, V>`
