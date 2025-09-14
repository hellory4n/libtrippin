#!/usr/bin/env python3
import os
import samurai.samurai as samurai # ???

cxx = "clang++"
cflags = "-std=c++17 -I. -I.. -Wall -Wextra -Wpedantic"
ldflags = "-lm -lstdc++"

mode = samurai.option("--mode", "debug or release")
if mode == "debug":
	cflags += " -O0 -g -DDEBUG -D_DEBUG"
elif mode == "release":
	cflags += " -O3 -g -fno-omit-frame-pointer"
elif mode != None:
	raise ValueError("rtfm dumbass")

crosscompile = samurai.option("--crosscompile", "only 'windows' works for now")
if crosscompile == "windows":
	cxx = "x86_64-w64-mingw32-g++"
elif crosscompile != None:
	raise ValueError("rtfm dumbass")

sanitize = samurai.option("--sanitize", "maps directly to a -fsanitize flag")
if sanitize != None:
	cflags += f" -fsanitize={sanitize}"
	ldflags += f" -fsanitize={sanitize}"

run = samurai.option("--run", "'true' to run executable, 'gdb' to run under gdb")
def postbuild():
	if run == "true":
		os.system("./build/bin/libtrippin")
	elif run == "gdb":
		os.system('gdb -q -ex run -ex "quit" --args ./build/bin/libtrippin')

samurai.project(samurai.Project(
	name = "libtrippin",
	compiler = cxx,
	cflags = cflags,
	ldflags = ldflags,
	postbuild = postbuild,
	sources = [
		"trippin/collection.cpp",
		"trippin/common.cpp",
		"trippin/error.cpp",
		"trippin/iofs.cpp",
		"trippin/log.cpp",
		"trippin/math.cpp",
		"trippin/memory.cpp",
		"trippin/string.cpp",
		"examples/test_all.cpp"
	],
))

samurai.run()
