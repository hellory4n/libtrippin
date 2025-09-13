# samurai: barely a build system®

Simple [Ninja](https://ninja-build.org) build script generator for Lua, because CMake and Make suck.

## Features

- No esoteric language
- Pretty simple
- Uhhhhhhhhhhh

## Limitations

- It's in alpha (experimental, will probabaly break, lacking features)
- No support for compilers other than GCC/Clang
- Probably doesn't work on Windows

## Usage

First you need Lua and Ninja installed (obviously).

Now copy `samurai.lua` and make a build script:

```lua
#!/bin/lua
local sam = require("samurai")
sam.init()

sam.project({
        name = "malware",
       	compiler = "gcc",
       	cflags = "-std=c99 -Wall -Wextra -Wpedantic",
       	ldflags = "-lm",

       	sources = {
               	"src/main.c",
       	},
})

sam.run()
```

This should be pretty simple to understand if you've done any C/C++ development ever.

A more advanced example would be:

```lua
local sam = require("samurai")
sam.init()

local cflags = "-std=c99 -Wall -Wextra -Wpedantic"
local ldflags = "-lm"

-- you can use cli options
sam.option("--mode", "debug or release", function(val)
       	if val == "debug" then
               	cflags = cflags.." -O0 -g"
       	elseif val == "release" then
                cflags = cflags.." -O2"
       	else
                error("rtfm you dastardly scoundrel")
       	end
end)

-- search for source files
local srcs = {}
local p = io.popen('find src/ -type f')
for file in p:lines() do
       	table.insert(srcs, file)
end

sam.project({
       	name = "malware",
       	compiler = "gcc",
       	cflags = cflags,
       	ldflags = ldflags,
       	sources = srcs
})

sam.run()
```

Make sure your build script is executable:

```sh
# you can name your build script anything idc
$ chmod +x build.lua
```

Now if you run it, you should see the help text:

```plaintext
$ ./build.lua
The samurai build system
usage: ./samurai.lua [command] [options...]

Commands:
help: shows this
configure: creates the ninja build script
build: builds the project
clean: removes all build files
version: prints the samurai + lua version
```

If you can't read, you have to `configure` once and then every time you edit your build script. `build` is literally just a tiny wrapper around running ninja.

It's that shrimple.

## FAQ

### Why?

the suffering is endless

### Didn't you already do this?

Kind of, I had [engineer](./engineerbuild/README.md) as an earlier version of the idea. The difference is that engineer handles the entire build process, while samurai is lower level and only generates build scripts for ninja to use.

### Isn't that just Premake?

It's similar, but Premake generates build scripts for a lot more build systems, which makes the whole system a lot more complicated. samurai instead entirely relies on you using a compiler like GCC or Clang, which is less portable, but GCC/Clang run everywhere, and it makes things a lot simpler and easier to use (at least for me). Also I don't like Premake.

### Have you tried \[build system] you fucking moron?

No fuck off.
