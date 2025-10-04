add_rules("mode.debug", "mode.release")

target("trippin")
	set_kind("static")
	set_languages("cxx20")
	set_warnings("allextra") -- -Wall -Wextra

	add_includedirs(".", {public = true})
	if is_plat("linux") then
		add_syslinks("m")
	end

	if is_mode("debug") then
		add_defines("DEBUG")
	end

	add_files(
		"trippin/common.cpp",
		"trippin/error.cpp",
		"trippin/iofs.cpp",
		"trippin/log.cpp",
		"trippin/math.cpp",
		"trippin/memory.cpp",
		"trippin/string.cpp",
		"trippin/util.cpp"
	)
target_end()

-- TODO xmake has a test system
-- ...so use that
target("testingit")
	set_kind("binary")
	set_warnings("allextra") -- -Wall -Wextra
	set_languages("cxx20")

	add_deps("trippin")

	if is_mode("debug") then
		add_defines("DEBUG")
	end

	add_files("examples/*.cpp")
target_end()
