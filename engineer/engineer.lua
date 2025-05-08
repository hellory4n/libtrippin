eng = require("libengineer")
eng.init()

project = eng.newproj("sir™", "executable")
project:add_cflags("-Wall -Weverything -Wpedantic -Wshadow")
project:add_ldflags("-L. -Wl,-rpath=.")
project:link({"m"})

project:add_includes({"."})
project:add_sources({"main.c"})

eng.recipe("build", function()
	project:build()
end)

eng.recipe("clean", function()
	project:clean()
end)

eng.util.print_table(project)

eng.run()
