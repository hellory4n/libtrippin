# samurai

Simple [Ninja](https://ninja-build.org) build script generator for Lua, because CMake and Make suck.

## Features

- No esoteric language
- Pretty simple
- Uhhhhhhhhhhh

## Limitations

- It's in alpha (experimental, will probabaly break, lacking features)
- No support for compilers other than GCC/Clang
- Probably doesn't work on Windows

usage:
```
> ./samurai.lua
running from build.lua
options: ...
> ./samurai.lua option1=yeah option2=fuck
configured :)
> ./samurai.lua build
oughhhh im building it
> ./samurai.lua clean
oughhhh im cleaning it
```

build script:
```lua
local sam = require("samurai")

sam.option("name", "description", function(val)
        -- do something with it
end)

-- do your usual build fuckery here

sam.project({
        name = "cocky",
        cflags = cflags,
        ldflags = ldflags,
        sources = srcs,
        target = "cocky",
        pre_build = function() --[[ ... ]] end
        post_build = function() --[[ ... ]] end
})
```

## FAQ

### Why?

the suffering is endless

### Didn't you already do this?

Kind of, I had [engineer](./engineerbuild/README.md) as an earlier version of the idea. The difference is that engineer handles the entire build process, while samurai is lower level and only generates build scripts for ninja to use.

### Isn't that just Premake?

It's similar, but Premake generates build scripts for a lot more build systems, which makes the whole system a lot more complicated. samurai instead entirely relies on you using a compiler like GCC or Clang, which is less portable, but GCC/Clang run everywhere, and it makes things a lot simpler and easier to use (at least for me). Also I don't like Premake.

### Have you tried \[build system] you fucking moron?

No fuck off.
