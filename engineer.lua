libtrippin = {}

local eng = require("engineerbuild.libengineer")
eng.init()

-- Returns the libtrippin project as a static library. Intended to be used for projects that use libtrippin.
-- Just link with "trippin" and build this before your own projects. `debug` is a bool. On linux you have to
-- link with "m" too. `trippinsrc` is where you put libtrippin.cpp
function libtrippin.lib(debug, trippinsrc)
	-- just trippin so it doesn't become "liblibtrippin.a"
	local project = eng.newproj("trippin", "staticlib", "c++14")
	project:pedantic()
	if debug then
		project:debug()
		project:optimization(0)
		project:define({"DEBUG"})
	else
		project:optimization(2)
	end

	project:add_sources({trippinsrc})
	project:target("libtrippin.a")
	return project
end
local trippin = libtrippin.lib(true, "libtrippin.cpp")

-- example projects :(
local example_all = eng.newproj("example_all", "executable", "c++14")
example_all:pedantic()
example_all:debug()
example_all:define({"DEBUG"})
example_all:link({"trippin", "m", "stdc++"})
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

eng.recipe("build-lib", "Builds only the library", function()
	trippin:build()
end)

eng.recipe("build", "Builds the library and the examples", function()
	eng.run_recipe("build-lib")
	example_all:build()
end)

eng.recipe("run-one-test-to-rule-them-all", "Runs the example with everything ever", function()
	eng.run_recipe("build")
	-- padding
	print()
	os.execute("./build/bin/example_all")
end)

eng.recipe("clean", "Cleans the project", function()
	trippin:clean()
	example_all:clean()
end)

eng.run()

return libtrippin
