eng = require("libengineer")
eng.init()

project = eng.newproj("sir™", "executable")
project:add_cflags("-Wall -Weverything -Wpedantic -Wshadow")
project:add_ldflags("-L. -Wl,-rpath=.")
project:link_dynamic({"m"})

project:add_includes({"."})
project:add_sources({"src/main.c", "src/test.c"})

eng.option("crosscomp", "Compiles to another platform. Only supported option is \"windows\"", function(val)
	-- we only support 2 platforms
	assert(val == "windows")
	eng.cc = "x86_64-w64-mingw32-gcc"
	eng.cxx = "x86_64-w64-mingw32-g++"
end)

eng.recipe("build", "Builds the project", function()
	project:build()
end)

eng.recipe("clean", "Cleans the binaries and stuff", function()
	project:clean()
end)

eng.run()
