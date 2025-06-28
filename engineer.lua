libtrippin = {}

local eng = require("libengineer")
eng.init()
eng.workstation("libtrippin", "hellory4n", "Most biggest most massive library of all time")

-- Returns the libtrippin project as a static library. Intended to be used for projects that use libtrippin.
-- Just link with "trippin" and build this before your own projects. `debug` is a bool. On linux you have to
-- link with "m" too. `trippindir` is the directory with libtrippin
function libtrippin.lib(debug, trippinsrc)
	-- just trippin so it doesn't become "liblibtrippin.a"
	local project = eng.newproj("trippin", "staticlib", "c++17")
	project:pedantic()
	if debug then
		project:debug()
		project:optimization(0)
		project:define({"DEBUG"})
	else
		project:optimization(2)
	end

	project:add_sources({
		trippinsrc.."/common.cpp",
		trippinsrc.."/log.cpp",
		trippinsrc.."/math.cpp",
		trippinsrc.."/memory.cpp",
		trippinsrc.."/string.cpp",
		trippinsrc.."/collection.cpp",
		trippinsrc.."/iofs.cpp",
	})
	project:target("libtrippin.a")
	project:define({"BACKWARD_HAS_BFD=1"})
	return project
end
local trippin = libtrippin.lib(true, "trippin")

-- example projects :(
-- TODO compile the other examples lmao
local example_all = eng.newproj("example_all", "executable", "c++17")
example_all:pedantic()
example_all:debug()
example_all:define({"DEBUG"})
-- idk why it doesn't link stdc++
example_all:link({"trippin", "bfd", "dl", "m", "stdc++"})
example_all:add_sources({"examples/one_test_to_rule_them_all.cpp"})
example_all:add_includes({"..", "."})
-- just one because it's pretty much the same for all of them
example_all:gen_compile_commands()

eng.option("debug", "true or false", function(val)
	if val == "true" then
		trippin:optimization(0)
		trippin:debug()
	end
end)

local asan = false
eng.option("asan", "if true, enables AddressSanitizer", function(val)
	if val == "true" then
		example_all:add_cflags("-fsanitize=address")
		example_all:add_ldflags("-fsanitize=address")
		asan = true
	end
end)

local breakpoint = ""
eng.option("breakpoint", "sets a breakpoint in gdb, e.g. \"breakpoint=src/file.c:43\"", function(val)
	breakpoint = val
end)

eng.recipe("build-lib", "Builds only the library", function()
	trippin:build()
end)

eng.recipe("build", "Builds the library and the examples", function()
	eng.run_recipe("build-lib")
	example_all:build()
end)

eng.recipe("run", "Runs the example with everything ever", function()
	eng.run_recipe("build")
	-- padding
	print()
	-- it could be just ./build/bin/example_all but then you would miss any panics that may happen
	if not asan then
		os.execute("gdb -q -ex \"break "..breakpoint.."\" -ex run -ex \"quit\" --args build/bin/example_all")
	else
		-- asan doesn't work with gdb
		os.execute("./build/bin/example_all")
	end
end)

eng.recipe("clean", "Cleans the project", function()
	trippin:clean()
	example_all:clean()
end)

eng.run()

return libtrippin
