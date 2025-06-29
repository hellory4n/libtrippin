--[[
	Engineer v1.2.0

	Bestest build system ever
	More information at https://github.com/hellory4n/libtrippin/tree/main/engineerbuild

	Copyright (C) 2025 by hellory4n <hellory4n@gmail.com>

	Permission to use, copy, modify, and/or distribute this
	software for any purpose with or without fee is hereby
	granted.

	THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS
	ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
	EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
	INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
	WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
	WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
	TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
	USE OR PERFORMANCE OF THIS SOFTWARE.
]]

-- bestest build system ever
eng = {
	-- why?
	util = {},
	-- C compiler
	cc = "",
	-- C++ compiler
	cxx = "",

	-- Table of strings with functions (no arguments, no returns)
	recipes = {},
	-- Table of strings and more strings
	recipe_description = {},
	-- Table of strings with functions (no arguments, no returns)
	options = {},
	-- Table of strings and more strings
	option_description = {},

	-- constants
	CONSOLE_COLOR_RESET = "\27[0m",
	CONSOLE_COLOR_LIB = "\27[0;90m",
	CONSOLE_COLOR_BLUE = "\27[0;34m",
	CONSOLE_COLOR_WARN = "\27[0;93m",
	CONSOLE_COLOR_ERROR = "\27[0;91m",

	-- Used to relink projects if anything changed since there's no dependency system lmao
	recompiling = false,
	-- man.
	workstation_name = "",
	workstation_description = "",
	workstation_author = "",
	-- changed with the showcmd option
	show_command = false,
}

-- Runs a command with no output
function eng.util.silentexec(command)
	-- TODO this will definitely break at some point
	return os.execute(command.." > /dev/null 2>&1")
end

-- Prints a table for debugging. This entire function is stolen.
function eng.util.print_table(node)
	local cache, stack, output = {},{},{}
	local depth = 1
	local output_str = "{\n"

	while true do
		local size = 0
		for k,v in pairs(node) do
			size = size + 1
		end

		local cur_index = 1
		for k,v in pairs(node) do
			if (cache[node] == nil) or (cur_index >= cache[node]) then

				if (string.find(output_str,"}",output_str:len())) then
					output_str = output_str..",\n"
				elseif not (string.find(output_str,"\n",output_str:len())) then
					output_str = output_str.."\n"
				end

				-- This is necessary for working with HUGE tables otherwise we run out of memory using concat on
				-- huge strings
				table.insert(output,output_str)
				output_str = ""

				local key
				if (type(k) == "number" or type(k) == "boolean") then
					key = "["..tostring(k).."]"
				else
					key = "['"..tostring(k).."']"
				end

				if (type(v) == "number" or type(v) == "boolean") then
					output_str = output_str..string.rep('\t',depth)..key.." = "..tostring(v)
				elseif (type(v) == "table") then
					output_str = output_str..string.rep('\t',depth)..key.." = {\n"
					table.insert(stack,node)
					table.insert(stack,v)
					cache[node] = cur_index+1
					break
				else
					output_str = output_str..string.rep('\t',depth)..eng.CONSOLE_COLOR_RESET..key..eng.CONSOLE_COLOR_RESET.." = '"..tostring(v).."'"
				end

				if (cur_index == size) then
					output_str = output_str.."\n"..string.rep('\t',depth-1).."}"
				else
					output_str = output_str..","
				end
			else
				-- close the table
				if (cur_index == size) then
					output_str = output_str.."\n"..string.rep('\t',depth-1).."}"
				end
			end

			cur_index = cur_index + 1
		end

		if (size == 0) then
			output_str = output_str.."\n"..string.rep('\t',depth-1).."}"
		end

		if (#stack > 0) then
			node = stack[#stack]
			stack[#stack] = nil
			depth = cache[node] == nil and depth + 1 or depth - 1
		else
			break
		end
	end

	-- This is necessary for working with HUGE tables otherwise we run out of memory using concat on huge strings
	table.insert(output,output_str)
	output_str = table.concat(output)

	print(output_str)
end

function eng.util.endswith(str, suffix)
	return str:sub(-#suffix) == suffix
end

-- As the name implies, it gets a file's checksum.
function eng.util.get_checksum(file)
	local fh = assert(io.popen("sha256sum "..file, "r"))
	local out = fh:read("*a")
	fh:close()
	-- take only the hex faffery
	return out:match("^(%S+)")
end

--
-- json.lua
--
-- Copyright (c) 2020 rxi
--
-- Permission is hereby granted, free of charge, to any person obtaining a copy of
-- this software and associated documentation files (the "Software"), to deal in
-- the Software without restriction, including without limitation the rights to
-- use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
-- of the Software, and to permit persons to whom the Software is furnished to do
-- so, subject to the following conditions:
--
-- The above copyright notice and this permission notice shall be included in all
-- copies or substantial portions of the Software.
--
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
-- IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
-- FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
-- AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
-- LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
-- OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
-- SOFTWARE.
--

local json = { _version = "0.1.2" }

-------------------------------------------------------------------------------
-- Encode
-------------------------------------------------------------------------------

local encode

local escape_char_map = {
  [ "\\" ] = "\\",
  [ "\"" ] = "\"",
  [ "\b" ] = "b",
  [ "\f" ] = "f",
  [ "\n" ] = "n",
  [ "\r" ] = "r",
  [ "\t" ] = "t",
}

local escape_char_map_inv = { [ "/" ] = "/" }
for k, v in pairs(escape_char_map) do
  escape_char_map_inv[v] = k
end


local function escape_char(c)
  return "\\" .. (escape_char_map[c] or string.format("u%04x", c:byte()))
end


local function encode_nil(val)
  return "null"
end


local function encode_table(val, stack)
  local res = {}
  stack = stack or {}

  -- Circular reference?
  if stack[val] then error("circular reference") end

  stack[val] = true

  if rawget(val, 1) ~= nil or next(val) == nil then
    -- Treat as array -- check keys are valid and it is not sparse
    local n = 0
    for k in pairs(val) do
      if type(k) ~= "number" then
        error("invalid table: mixed or invalid key types")
      end
      n = n + 1
    end
    if n ~= #val then
      error("invalid table: sparse array")
    end
    -- Encode
    for i, v in ipairs(val) do
      table.insert(res, encode(v, stack))
    end
    stack[val] = nil
    return "[" .. table.concat(res, ",") .. "]"

  else
    -- Treat as an object
    for k, v in pairs(val) do
      if type(k) ~= "string" then
        error("invalid table: mixed or invalid key types")
      end
      table.insert(res, encode(k, stack) .. ":" .. encode(v, stack))
    end
    stack[val] = nil
    return "{" .. table.concat(res, ",") .. "}"
  end
end


local function encode_string(val)
  return '"' .. val:gsub('[%z\1-\31\\"]', escape_char) .. '"'
end


local function encode_number(val)
  -- Check for NaN, -inf and inf
  if val ~= val or val <= -math.huge or val >= math.huge then
    error("unexpected number value '" .. tostring(val) .. "'")
  end
  return string.format("%.14g", val)
end


local type_func_map = {
  [ "nil"     ] = encode_nil,
  [ "table"   ] = encode_table,
  [ "string"  ] = encode_string,
  [ "number"  ] = encode_number,
  [ "boolean" ] = tostring,
}


encode = function(val, stack)
  local t = type(val)
  local f = type_func_map[t]
  if f then
    return f(val, stack)
  end
  error("unexpected type '" .. t .. "'")
end


function json.encode(val)
  return ( encode(val) )
end

-- Initializes engineer™
function eng.init()
	-- reset state bcuz modules are obnoxious :)
	eng.cc = ""
	eng.cxx = ""
	eng.recipes = {}
	eng.recipe_description = {}
	eng.options = {}
	eng.option_description = {}
	eng.recompiling = false

	eng.option("showcmd", "If true, prints the compiler commands being used", function(val)
		eng.show_command = true
	end)

	eng.option("cc", "Sets the C compiler. Default is clang", function(val)
		eng.cc = val
	end)

	eng.option("cxx", "Sets the C++ compiler. Default is clang++", function(val)
		eng.cxx = val
	end)

	-- default help recipe
	eng.recipe("help", "Shows what you're seeing right now", function()
		-- if theres no eng.workstation() just shut up
		if eng.workstation_name ~= "" then
			print(eng.workstation_name.." by "..eng.workstation_author)
			print(eng.workstation_description.."\n")
		end

		print("Recipes:")

		-- some sorting lamo
		local rkeys = {}
		for recipe, _ in pairs(eng.recipe_description) do
			table.insert(rkeys, recipe)
		end
		table.sort(rkeys)

		-- actually list the shitfuck
		for _, recipe in ipairs(rkeys) do
			print(string.format("%-20s", "- "..recipe..": ")..eng.recipe_description[recipe])
		end

		-- mate
		print("\nOptions: (usage: <option>=<value>)")
		local okeys = {}
		for option, _ in pairs(eng.option_description) do
			table.insert(okeys, option)
		end
		table.sort(okeys)

		for _, option in ipairs(okeys) do
			print(string.format("%-20s", "- "..option..": ")..eng.option_description[option])
		end

		print("\nEngineer v1.2.0 - copyright (C) 2025 hellory4n")
	end)
end

-- Sets additional information to be displayed in the help recipe
function eng.workstation(name, author, description)
	eng.workstation_name = name
	eng.workstation_author = author
	eng.workstation_description = description
end

-- Makes a recipe. The callback will be called when the recipe is used. The description will be used for
-- the default help recipe.
function eng.recipe(name, description, callback)
	eng.recipes[name] = callback
	eng.recipe_description[name] = description
end

-- Adds an option. The callback takes in whatever value the option has (string), and is only called
-- if that option is used. The description will be used for the default help recipe.
function eng.option(name, description, callback)
	eng.options[name] = callback
	eng.option_description[name] = description
end

-- Put this at the end of your build script so it actually does something
function eng.run()
	local opts = {}
	local recipes = {}

	for _, argma in ipairs(arg) do
		-- TODO this will probably break
		local key, val = argma:match("^([%w%-_]+)=?(%S*)$")
		-- man.
		if key == "" then key = nil end
		if val == "" then val = nil end

		-- we have to sort options and recipes so options are set before recipes
		if key and val then
			table.insert(opts, {key = key, val = val})
		elseif key then
			table.insert(recipes, key)
		end
	end

	for _, mate in ipairs(opts) do
		if eng.options[mate.key] ~= nil then
			eng.options[mate.key](mate.val)
		else
			print(eng.CONSOLE_COLOR_WARN.."unknown option \""..mate.key.."\""..eng.CONSOLE_COLOR_RESET)
		end
	end

	for _, recipema in ipairs(recipes) do
		if eng.recipes[recipema] ~= nil then
			eng.recipes[recipema]()
		else
			print(eng.CONSOLE_COLOR_WARN.."unknown recipe \""..recipema.."\""..eng.CONSOLE_COLOR_RESET)
		end
	end
end

-- Forces a recipe to run
function eng.run_recipe(recipe)
	eng.recipes[recipe]()
end

function eng.check_compiler()
	if eng.cc == "" then
		-- get c compiler
		local cc = os.getenv("CC")
		if cc ~= nil and cc ~= "" then
			eng.cc = cc
		else
			if eng.util.silentexec("command -v clang") then
				eng.cc = "clang"
			elseif eng.util.silentexec("command -v gcc") then
				eng.cc = "gcc"
			else
				error(eng.CONSOLE_COLOR_ERROR ..
					"no C compiler found. please install gcc or clang, and make sure it's in the PATH" ..
					eng.CONSOLE_COLOR_RESET)
			end
		end
	end

	if eng.cxx == "" then
		-- get c++ compiler
		local cxx = os.getenv("CXX") or os.getenv("CPP")
		if cxx ~= nil and cxx ~= "" then
			eng.cxx = cxx
		else
			if eng.util.silentexec("command -v clang++") then
				eng.cxx = "clang++"
			elseif eng.util.silentexec("command -v g++") then
				eng.cxx = "g++"
			else
				error(eng.CONSOLE_COLOR_ERROR ..
					"no C++ compiler found. please install gcc or clang, and make sure it's in the PATH" ..
					eng.CONSOLE_COLOR_RESET)
			end
		end
	end
end

-- project metatable
local project_methods = {}
local project = {
	__index = project_methods,
}

-- Creates a new project. The type can be either "executable", "sharedlib", or "staticlib". The standard
-- is all lowercase, e.g. c99, c++11
function eng.newproj(name, type, std)
	assert(type == "executable" or type == "sharedlib" or type == "staticlib",
		eng.CONSOLE_COLOR_ERROR.."type must be executable, sharedlib, or staticlib"..eng.CONSOLE_COLOR_RESET)

	local t = setmetatable({}, project)
	t.name = name
	t.type = type
	t.builddir = "build"
	t.cflags = " -std="..std.." "
	-- so static and shared libraries work :)
	-- help
	-- TODO this will break
	t.ldflags = "-L. -Lbuild/static -Lbuild/bin -L../build/static -L../build/bin -Wl,-rpath=. -Wl,-rpath=./build/bin "
	t.sources = {}
	t.targetma = name
	return t
end

-- Adds compile flags to the project. It's recommended to use project methods instead of manually adding
-- flags wherever possible.
function project_methods.add_cflags(proj, cflags)
	proj.cflags = proj.cflags.." "..cflags.." "
end

-- Adds linker flags to the project. It's recommended to use project methods instead of manually adding
-- flags wherever possible.
function project_methods.add_ldflags(proj, ldflags)
	proj.ldflags = proj.ldflags.." "..ldflags
end

-- Links multiple libraries to the project (it's a list). This shouldn't have any prefixes/suffixes, so
-- for example use "trippin" instead of "libtrippin", "trippin.dll", "libtrippin.a", etc
function project_methods.link(proj, libs)
	for _, lib in ipairs(libs) do
		proj.ldflags = proj.ldflags.." -l"..lib.." "
	end
end

-- Adds multiple source files to the project (it's a list).
function project_methods.add_sources(proj, srcs)
	for _, src in ipairs(srcs) do
		table.insert(proj.sources, src)
	end
end

-- Adds multiple include directories to the project (it's a list).
function project_methods.add_includes(proj, incs)
	for _, inc in ipairs(incs) do
		proj.cflags = proj.cflags.." -I"..inc.." "
	end
end

-- Sets the project's build directory. By default this is "build"
--function project_methods.build_dir(proj, dir)
--	proj.build_dir = dir
--	proj.ldflags = proj.ldflags.."-L"..proj.builddir.."/static -L"..proj.builddir.."/bin"
--end

-- Sets the project's target. e.g. crapplication.exe, libfaffery.so, etc
function project_methods.target(proj, target)
	proj.targetma = target
end

-- Adds common flags to make the compiler more obnoxious
function project_methods.pedantic(proj)
	proj.cflags = proj.cflags.." -Wall -Wextra -Wpedantic "
end

-- Enables debug info
function project_methods.debug(proj)
	proj.cflags = proj.cflags.." -g "
end

-- Sets the optimization level, from 0 (no optimization) to 3 (max optimization)
function project_methods.optimization(proj, level)
	if level == 0 then proj.cflags = proj.cflags.." -O0 "
	elseif level == 1 then proj.cflags = proj.cflags.." -O1 "
	elseif level == 2 then proj.cflags = proj.cflags.." -O2 "
	elseif level == 3 then proj.cflags = proj.cflags.." -O3 "
	else
		error(eng.CONSOLE_COLOR_ERROR.."you bloody scoundrel that's not a valid level"..eng.CONSOLE_COLOR_RESET)
	end
end

-- Adds multiple defines (it's a list) to the project
function project_methods.define(proj, defines)
	for _, def in ipairs(defines) do
		proj.cflags = proj.cflags.." -D"..def.." "
	end
end

-- Returns a list of source files that changed.
function project_methods.get_changed_files(proj)
	local cachepath = proj.builddir.."/"..proj.name..".cache"

	-- parse old cache
	local old = {}
	do
		local f = io.open(cachepath, "r")
		if f then
		for line in f:lines() do
			local sep = line:find("=")
			if sep then
			local src = line:sub(1, sep-1)
			local sum = line:sub(sep+1)
			old[src] = sum
			end
		end
		f:close()
		end
	end

	-- check if anything changed
	local changed = {}
	local newcache = {}

	for _, src in ipairs(proj.sources) do
		local newsum = eng.util.get_checksum(src)
		table.insert(newcache, src.."="..newsum)
		if old[src] ~= newsum then
		table.insert(changed, src)
		end
	end

	-- make a new cache
	local f, err = io.open(cachepath, "w")
	assert(f, err)
	f:write(table.concat(newcache, "\n"), "\n")
	f:close()

	-- if there was no old cache then everythings new
	if next(old) == nil then
		return proj.sources
	end

	return changed
end

-- Builds and links the entire project
function project_methods.build(proj)
	eng.check_compiler()
	print("Compiling "..proj.name.." with "..eng.cc.."/"..eng.cxx)

	-- folder? i hardly know 'er!
	eng.util.silentexec("mkdir "..proj.builddir)
	eng.util.silentexec("mkdir "..proj.builddir.."/obj")
	eng.util.silentexec("mkdir "..proj.builddir.."/static")
	eng.util.silentexec("mkdir "..proj.builddir.."/bin")

	-- just in case
	eng.util.silentexec("rm "..proj.builddir.."/.buildjob.lua")
	eng.util.silentexec("rm "..proj.builddir.."/.fail")

	-- compile the bloody files
	local srcs = proj:get_changed_files()
	-- na
	if #srcs == 0 then
		print(proj.name.." is already up to date; nothing to do")
	else
		eng.recompiling = true
	end

	for _, src in ipairs(srcs) do
		local fuck = {
			src = src,
			cc = eng.cc,
			cxx = eng.cxx,
			show_command = eng.show_command,
			proj = proj,
		}
		local fuckjson = json.encode(fuck):gsub("\"", "\\\"")

		-- lua doenst fucking have fuvcking threads or tasks
		-- so we're just making malware then launching it with & at the end, which makes it run in the background
		-- it's different on windows but i'm not gonna fucking do that
		-- man.
		local success = os.execute("ENGINEER_TEMP_PROJ=\""..fuckjson.."\" lua libengineertask.lua &")
		assert(success, "unexpected internal error")
	end

	if not eng.recompiling then return end

	-- get objs
	local objs = {}
	for _, src in ipairs(proj.sources) do
		table.insert(objs, proj.builddir.."/obj/"..src:gsub("/", "@")..".o")
	end

	-- all the compile commands are running on the background
	-- so we have to wait for all the obj files to show up
	local expected_objs = #objs
	while true do
		-- did compilation fail?
		local failf = io.open(proj.builddir.."/.fail", "r")
		if failf ~= nil then
			error(eng.CONSOLE_COLOR_ERROR.."compiling "..proj.name.." failed"..eng.CONSOLE_COLOR_RESET)
		end

		-- check for the files
		local obj_count = 0
		for _, obj in ipairs(objs) do
			local objf = io.open(obj, "r")
			if objf ~= nil then obj_count = obj_count + 1 end
		end

		if obj_count >= expected_objs then
			break
		end
	end

	-- link :)
	-- first get the object files
	local objma = ""
	for _, src in ipairs(proj.sources) do
		objma = objma..proj.builddir.."/obj/"..src:gsub("/", "@")..".o "
	end

	-- link executable
	if proj.type == "executable" then
		print("Linking "..proj.name)
		-- the ldflags must come last lamo
		local cmd = eng.cc.." "..objma.." -o "..proj.builddir.."/bin/"..proj.targetma.." "..proj.ldflags
		if eng.show_command then
			print(cmd)
		end

		local success = os.execute(cmd)
		if not success then
			error(eng.CONSOLE_COLOR_ERROR.."linking "..proj.name.." failed"..eng.CONSOLE_COLOR_RESET)
		end
	end

	-- link static library
	if proj.type == "staticlib" then
		print("Linking "..proj.name)
		local cmd = "ar rcs "..proj.builddir.."/static/"..proj.targetma.." "..objma
		if eng.show_command then
			print(cmd)
		end
		os.execute(cmd)
	end

	-- link shared library
	if proj.type == "sharedlib" then
		-- the ldflags must come last lamo
		local cmd = eng.cc.." -shared "..objma.." -o "..proj.builddir.."/bin/"..proj.targetma.." "..proj.ldflags
		if eng.show_command then
			print(cmd)
		end

		local success = os.execute(cmd)
		if not success then
			error(eng.CONSOLE_COLOR_ERROR.."linking "..proj.name.." failed"..eng.CONSOLE_COLOR_RESET)
		end
	end

	print("Built "..proj.name.." successfully")
end

-- As the name implies, it cleans the project
function project_methods.clean(proj)
	-- Scary!
	os.execute("rm -rf "..proj.builddir)
end

-- Generates a compile_commands.json file for clangd to use
function project_methods.gen_compile_commands(proj)
	-- i won't even bother making it formatted
	local commands = "["
	for i, src in ipairs(proj.sources) do
		commands = commands.."{"

		-- lua doesn't have a function to get the working directory
		local fh = assert(io.popen("pwd", "r"))
		-- there's a newline and it makes json crash and die
		local cwd = fh:read("*a"):sub(1, -2)
		fh:close()

		-- i appreciate c++
		local compiler = ""
		if eng.util.endswith(src, ".c") then
			compiler = eng.cc
		elseif eng.util.endswith(src, ".cpp") or eng.util.endswith(src, ".cc") or
		eng.util.endswith(src, ".cxx") or eng.util.endswith(src, ".c++") then
			compiler = eng.cxx
		else
			print(eng.CONSOLE_COLOR_WARN.."unexpected extension in "..src..", using "..eng.cc..eng.CONSOLE_COLOR_RESET)
			compiler = eng.cc
		end

		-- man
		local obj = proj.builddir.."/obj/"..src:gsub("/", "_")..".o"
		commands = commands.."\"directory\": \""..cwd.."\",\"file\": \""..cwd.."/"..src.."\", \"command\": \""
		commands = commands..compiler..proj.cflags.." -o "..obj.." -c "..src
		commands = commands.."\"}"

		-- why tf doesn't json support trailing commas
		if i < #proj.sources then
			commands = commands..","
		end
	end
	commands = commands.."]"

	-- mate
	local f = io.open("compile_commands.json", "w")
	assert(f)
	f:write(commands)
	f:close()
end

return eng
