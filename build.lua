#!/bin/lua
local sam = require("samurai/samurai")
sam.init()

sam.option("--mycock", "MY COCK NOTATION!", function(val)
	print("MYCOCK " .. val)
end)

sam.project({
	name = "cock",
	compiler = "clang",
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
