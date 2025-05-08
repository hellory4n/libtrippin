eng = require("libengineer")
eng.init()

exeproj = eng.newproj("sir™", "executable")
--exeproj:add_cflags("-Wall -Weverything -Wpedantic -Wshadow")
exeproj:link_dynamic({"m", "staticma", "sharedma"})
exeproj:add_includes({"src"})
exeproj:add_sources({"src/main.c", "src/test.c"})

staticlib = eng.newproj("staticma", "staticlib")
--staticlib:add_cflags("-Wall -Weverything -Wpedantic -Wshadow")
staticlib:add_includes({"src"})
staticlib:add_sources({"src/staticlib.c"})
staticlib:target("libstaticma.a")

sharedlib = eng.newproj("sharedma", "sharedlib")
--sharedlib:add_cflags("-Wall -Weverything -Wpedantic -Wshadow")
sharedlib:add_includes({"src"})
sharedlib:add_sources({"src/sharedlib.c"})
sharedlib:target("libsharedma.so")

eng.option("crosscomp", "Compiles to another platform. Only supported option is \"windows\"", function(val)
	-- we only support 2 platforms
	assert(val == "windows")
	eng.cc = "x86_64-w64-mingw32-gcc"
	eng.cxx = "x86_64-w64-mingw32-g++"
end)

eng.recipe("build", "Builds the project", function()
	staticlib:build()
	sharedlib:build()
	exeproj:build()
end)

eng.recipe("clean", "Cleans the binaries and stuff", function()
	staticlib:clean()
	exeproj:clean()
end)

eng.recipe("run", "Builds and runs the project", function()
	eng.run_recipe("build")
	print("") -- separation
	os.execute("./build/bin/sir™")
end)

eng.run()
