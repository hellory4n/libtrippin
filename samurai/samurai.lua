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
---@field cflags string
---@field ldflags string
---@field sources string[]
---@field target string
---@field pre_build fun()
---@field post_build fun()
local Project = {}

local sam = {
	version = "v0.1.0",
	---@type table<string, fun(val: string)>
	_options = {},
	---@type table<string, string>
	_option_descriptions = {},
	---@type Project[]
	_projects = {},
}

function sam.init()
	-- TODO
end

cock = {}

--- Adds a command-line option, and calls the callback function if it's used.
---@param name string
---@param description string
---@param callback fun(val: string)
function sam.option(name, description, callback)

end

return sam
