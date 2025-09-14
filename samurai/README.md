# samurai: barely a build system®

Simple [Ninja](https://ninja-build.org) build script generator for Python, because CMake and Make suck.

## Features

- No esoteric language, only Python
- Pretty simple
- Uhhhhhhhhhhh

## Limitations

- It's in alpha (experimental, will probabaly break, lacking features)
- No support for compilers other than GCC/Clang
- Probably doesn't work on Windows
- You have to use Python

## Usage

First you need Python and Ninja installed (obviously).

Now copy `samurai.py` and make a build script:

```py
#!/usr/bin/env python3
import samurai

samurai.project(samurai.Project(
        name = "malware",
        compiler = "gcc",
        cflags = "-std=c99 -Wall -Wextra -Wpedantic",
        ldflags = "-lm",
        sources = [
                "src/main.c",
        ],
))

samurai.run()
```

This should be pretty simple to understand if you've done any C/C++ development ever.

A more advanced example would be:

```py
#!/usr/bin/env python3
import glob
import samurai

cflags = "-std=c99 -Wall -Wextra -Wpedantic"
ldflags = "-lm"

# you can use cli options
mode = samurai.option("--mode", "debug or release")
if mode == "debug":
	cflags += " -O0 -g -DDEBUG -D_DEBUG"
elif mode == "release":
	cflags += " -O3"
elif mode != None:
	raise ValueError("rtfm dumbass")

# a run command (spicy)
run = samurai.option("--run", "'true' to run executable, 'gdb' to run under gdb")
def postbuild():
	if run == "true":
		os.system("./build/bin/malware")
	elif run == "gdb":
		os.system('gdb -q -ex run -ex "quit" --args ./build/bin/malware')

# get srcs
srcs = glob.glob("src/**.c", recursive=True)

samurai.project(samurai.Project(
        name = "malware",
        compiler = "gcc",
        cflags = cflags,
        ldflags = ldflags,
        postbuild = postbuild,
        sources = srcs,
))

samurai.run()
```

Make sure your build script is executable:

```sh
# you can name your build script anything idc
$ chmod +x build.py
```

Now if you run it, you should see the help text:

```plaintext
$ ./build.py
The samurai build system
Usage: ./build.py [command] [options...]

Commands:
    help: shows this
    configure: creates the ninja build script
    build: builds the project
    clean: removes all build files
    version: prints the samurai + python version
```

If you can't read, you have to `configure` once and then every time you edit your build script. `build` is literally just a tiny wrapper around running ninja.

It's that shrimple.

## FAQ

### Why?

the suffering is endless

### Didn't you already do this?

Kind of, I had [engineer](./engineerbuild/README.md) as an earlier version of the idea. The difference is that engineer handles the entire build process, while samurai is lower level and only generates build scripts for ninja to use. Also samurai uses Python instead of Lua.

### Have you tried \[build system] you fucking moron?

No fuck off.
