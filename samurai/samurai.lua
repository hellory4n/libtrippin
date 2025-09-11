--[[
-- samurai: Simple Ninja build script generator
-- https://github.com/hellory4n/libtrippin
--
-- Copyright (C) 2025 by hellory4n <hellory4n@gmail.com>
--
-- Permission to use, copy, modify, and/or distribute this
-- software for any purpose with or without fee is hereby
-- granted.
--
-- THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS
-- ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
-- IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
-- EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
-- INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
-- WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
-- WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
-- TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
-- USE OR PERFORMANCE OF THIS SOFTWARE.
]]

-- if you're wondering what all the funny ---@ things are,
-- they're annotations for the [lua lsp](https://github.com/LuaLS/lua-language-server)
-- mostly type annotations

---@class (exact) Project
---@field name string
---@field compiler string
---@field cflags string? - flags used during compilation
---@field ldflags string? - flags used during linking
---@field sources string[] - list of source files
---@field target string? - the final name of the executable
---@field pre_build fun()? - a function to run before building the project
---@field post_build fun()? - a function to run after building the project
local Project = {}

local sam = {
	version = "v0.1.0",

	_cli_project_name = "",
	_cli_author = "",
	_cli_license = "",
	_cli_url = "",

	---@type table<string, fun(val: string)>
	_options = {},
	---@type table<string, string>
	_option_descriptions = {},
	---@type Project?
	_project = nil,
}

--- Adds a command-line option, and calls the callback function if it's used.
---@param name string
---@param description string
---@param callback fun(val: string)
function sam.option(name, description, callback)
	sam._options[name] = callback
	sam._option_descriptions[name] = description
end

--- Makes sure nothing tragically fails trying to access a nil field
---@param project Project
function sam._project_defaults(project)
	if project == nil then
		error("Please set a project with 'sam.project()'")
	end

	-- required fields
	if project.name == nil or project.name == "" then
		error("Please give your project a name with the '.name' field.")
	end
	if project.compiler == nil or project.compiler == "" then
		error("Please give your project a compiler with the '.compiler' field.")
	end
	if project.sources == nil or #project.sources == 0 then
		error("Your project needs at least 1 source file. (see the '.sources' option)")
	end

	-- optional fields
	if project.target == nil or project.target == "" then
		project.target = project.name
	end
	if project.cflags == nil then
		project.cflags = ""
	end
	if project.ldflags == nil then
		project.ldflags = ""
	end
	if project.pre_build == nil then
		project.pre_build = function() end
	end
	if project.post_build == nil then
		project.post_build = function() end
	end
end

--- Makes a new project. Note that there can only be one project. See [Project](lua://Project) for
--- available options.
---@param project Project
function sam.project(project)
	sam._project_defaults(project)
	sam._project = project
end

--- Initializes the library
function sam.init()
	-- TODO
end

function sam._help()
	print("hehe")
end

function sam._configure()
	print("oughhh im configuring it")
end

function sam._build()
	print("oughhh im building it")
end

--- Put this at the end of your build script to actually do anything
function sam.run()
	if sam._project == nil then
		error("Please set a project with 'sam.project()'")
	end

	local opts = {}

	for i, argma in ipairs(arg) do
		-- first arg is the command
		if i == 1 then
			goto continue
		end

		-- TODO this will probably break
		local key, val = argma:match("^([%w%-_]+)=?(%S*)$")
		if key == "" then key = nil end
		if val == "" then val = nil end

		if key and val then
			table.insert(opts, { key = key, val = val })
		else
			-- TODO this is a horrible error message
			error("expected value")
		end

		::continue::
	end

	for _, mate in ipairs(opts) do
		if sam._options[mate.key] ~= nil then
			sam._options[mate.key](mate.val)
		else
			warn("uknown option '" .. mate.key .. "'")
		end
	end

	-- arg[1] should be the command
	if arg[1] == "help" or arg[1] == "--help" or arg[1] == "-h" or arg[1] == nil then
		sam._help()
	elseif arg[1] == "configure" then
		sam._configure()
	elseif arg[1] == "build" then
		sam._build()
	else
		error("invalid command '" .. arg[1] .. "' (try 'help' to see available commands)")
	end
end

return sam
