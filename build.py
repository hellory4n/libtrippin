#!/usr/bin/env python3
import samurai.samurai as samurai # ???
samurai.init()

cxx = "clang++"
cflags = "-std=c++17 -I. -I.. -Wall -Wextra -Wpedantic"
ldflags = "-lm -lstdc++"

samurai.option("--mode", "debug or release", lambda val:
	cflags = if val: cflags + "" else cflags + "")
