#!/bin/lua
local sam = require("samurai/samurai")
sam.init()

cxx = "clang++"
cflags = "-std=c++17 -I. -I.. -Wall -Wextra -Wpedantic"
ldflags = "-lm -lstdc++"

sam.option("--mode", "debug or release", function(val)
	if val == "debug" then
		cflags = cflags .. " -O0 -g -DDEBUG -D_DEBUG"
		ldflags = ldflags .. " -MYCOCK"
	elseif val == "release" then
		cflags = cflags .. " -O3 -g -fno-omit-frame-pointer"
	else
		error("rtfm you dastardly scoundrel")
	end
	assert(_G["cflags"] == nil)
	sam.print_table(_G)
	print(cflags)
end)

sam.option("--crosscompile", "only 'windows' works for now", function(val)
	if val == "windows" then
		cxx = "x86_64-w64-mingw32-g+"
	else
		error("rtfm you dastardly scoundrel")
	end
end)

sam.option("--sanitize", "maps directly to a -fsanitize flag", function(val)
	cflags = cflags .. " -fsanitize=" .. val
	ldflags = ldflags .. " -fsanitize=" .. val
end)

sam.project({
	name = "libtrippin",
	compiler = cxx,
	cflags = cflags,
	ldflags = ldflags,

	sources = {
		"trippin/collection.cpp",
		"trippin/common.cpp",
		"trippin/error.cpp",
		"trippin/iofs.cpp",
		"trippin/log.cpp",
		"trippin/math.cpp",
		"trippin/memory.cpp",
		"trippin/string.cpp",
		"examples/test_all.cpp"
	},
})

sam.run()
