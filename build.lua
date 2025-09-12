#!/bin/lua
local sam = require("samurai/samurai")
sam.init()

sam.option("--mycock", "MY COCK NOTATION!", function(val)
	print("MYCOCK " .. val)
end)

sam.project({
	name = "cock",
	compiler = "clang",
	-- insanity
	cflags =
	"-std=c++17 -O3 -g -DDEBUG -D_DEBUG -I. -I.. -Wall -Wextra -Wpedantic -Wuninitialized -Wshadow -Wconversion -Wold-style-cast -Wextra-semi -Wmissing-noreturn -Wimplicit-fallthrough -Wnull-dereference -Wcast-qual",
	ldflags = "-lm -stdc++",

	sources = {
		"trippin/collection.cpp",
		"trippin/common.cpp",
		"trippin/error.cpp",
		"trippin/iofs.cpp",
		"trippin/log.cpp",
		"trippin/math.cpp",
		"trippin/memory.cpp",
		"trippin/string.cpp",
		""
	}
})

sam.run()
