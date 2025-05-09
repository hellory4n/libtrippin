# Engineer™

Got so annoyed at Make that I made my own build system.

This works by pretty much just being Make, but in Lua. (no esoteric language)

All of the C files are just for testing, they're not part of Engineer™

## Features

- No esoteric language
- Only external dependency is lua
- You can specify options and stuff
- You can control the entire build process
- Incremental builds
- Support for clangd's `compile_commands.json`

## Limitations

- Linux only
- GCC/Clang only
- You probably can't use a massive IDE with it
- It's made by some random guy

## FAQ

### Isn't that just Premake?

No they're quite different, the only thing in common is Lua.

### Have you tried \[build system] you fucking moron?

No fuck off.
