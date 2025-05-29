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
	else
		project:optimization(2)
	end

	project:add_sources({trippinsrc})
	project:target("libtrippin.a")
	return project
end
local trippin = libtrippin.lib(false, "libtrippin.cpp")

-- example projects :(
local example_log = eng.newproj("example_log", "executable", "c++11")
example_log:pedantic()
example_log:debug()
example_log:link({"trippin", "m"})
example_log:add_sources({"examples/log.cpp"})
example_log:add_includes({"..", "."})
-- just one because it's pretty much the same for all of them
example_log:gen_compile_commands()

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
	example_log:build()
end)

eng.recipe("run-log", "Runs the log example", function()
	eng.run_recipe("build")
	os.execute("./build/bin/example_log")
end)

eng.recipe("clean", "Cleans the project", function()
	trippin:clean()
	example_log:clean()
end)

eng.run()

return libtrippin
