eng = require("libengineer")
eng.init()

project = eng.newproj("sir™", "executable")
project:add_cflags("-Wall -Weverything -Wpedantic -Wshadow")
project:add_ldflags("-L. -Wl,-rpath=.")
project:link_dynamic({"m"})

project:add_includes({"."})
project:add_sources({"main.c"})

eng.option("crosscomp", function(val)
	-- we only support 2 platforms
	assert(val == "windows")
	eng.cc = "x86_64-w64-mingw32-gcc"
	eng.cxx = "x86_64-w64-mingw32-g++"
end)

eng.recipe("build", function()
	project:build()
end)

eng.recipe("clean", function()
	project:clean()
end)

eng.run()
