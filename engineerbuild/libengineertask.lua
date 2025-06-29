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

eng = {
	-- why?
	util = {},

	-- constants
	CONSOLE_COLOR_RESET = "\27[0m",
	CONSOLE_COLOR_LIB = "\27[0;90m",
	CONSOLE_COLOR_COMPILE = "\27[0;32m",
	CONSOLE_COLOR_WARN = "\27[0;93m",
	CONSOLE_COLOR_ERROR = "\27[0;91m",
}

function eng.util.endswith(str, suffix)
	return str:sub(-#suffix) == suffix
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
-- Decode
-------------------------------------------------------------------------------

local parse

local function create_set(...)
	local res = {}
	for i = 1, select("#", ...) do
		res[ select(i, ...) ] = true
	end
	return res
end

local space_chars   = create_set(" ", "\t", "\r", "\n")
local delim_chars   = create_set(" ", "\t", "\r", "\n", "]", "}", ",")
local escape_chars  = create_set("\\", "/", '"', "b", "f", "n", "r", "t", "u")
local literals      = create_set("true", "false", "null")

local literal_map = {
	[ "true"  ] = true,
	[ "false" ] = false,
	[ "null"  ] = nil,
}


local function next_char(str, idx, set, negate)
	for i = idx, #str do
		if set[str:sub(i, i)] ~= negate then
			return i
		end
	end
	return #str + 1
end


local function decode_error(str, idx, msg)
	local line_count = 1
	local col_count = 1
	for i = 1, idx - 1 do
		col_count = col_count + 1
		if str:sub(i, i) == "\n" then
			line_count = line_count + 1
			col_count = 1
		end
	end
	error( string.format("%s at line %d col %d", msg, line_count, col_count) )
end


local function codepoint_to_utf8(n)
	-- http://scripts.sil.org/cms/scripts/page.php?site_id=nrsi&id=iws-appendixa
	local f = math.floor
	if n <= 0x7f then
		return string.char(n)
	elseif n <= 0x7ff then
		return string.char(f(n / 64) + 192, n % 64 + 128)
	elseif n <= 0xffff then
		return string.char(f(n / 4096) + 224, f(n % 4096 / 64) + 128, n % 64 + 128)
	elseif n <= 0x10ffff then
		return string.char(f(n / 262144) + 240, f(n % 262144 / 4096) + 128,
											 f(n % 4096 / 64) + 128, n % 64 + 128)
	end
	error( string.format("invalid unicode codepoint '%x'", n) )
end


local function parse_unicode_escape(s)
	local n1 = tonumber( s:sub(1, 4),  16 )
	local n2 = tonumber( s:sub(7, 10), 16 )
	 -- Surrogate pair?
	if n2 then
		return codepoint_to_utf8((n1 - 0xd800) * 0x400 + (n2 - 0xdc00) + 0x10000)
	else
		return codepoint_to_utf8(n1)
	end
end

local escape_char_map_inv = { [ "/" ] = "/" }

local function parse_string(str, i)
	local res = ""
	local j = i + 1
	local k = j

	while j <= #str do
		local x = str:byte(j)

		if x < 32 then
			decode_error(str, j, "control character in string")

		elseif x == 92 then -- `\`: Escape
			res = res .. str:sub(k, j - 1)
			j = j + 1
			local c = str:sub(j, j)
			if c == "u" then
				local hex = str:match("^[dD][89aAbB]%x%x\\u%x%x%x%x", j + 1)
								 or str:match("^%x%x%x%x", j + 1)
								 or decode_error(str, j - 1, "invalid unicode escape in string")
				res = res .. parse_unicode_escape(hex)
				j = j + #hex
			else
				if not escape_chars[c] then
					decode_error(str, j - 1, "invalid escape char '" .. c .. "' in string")
				end
				res = res .. escape_char_map_inv[c]
			end
			k = j + 1

		elseif x == 34 then -- `"`: End of string
			res = res .. str:sub(k, j - 1)
			return res, j + 1
		end

		j = j + 1
	end

	decode_error(str, i, "expected closing quote for string")
end


local function parse_number(str, i)
	local x = next_char(str, i, delim_chars)
	local s = str:sub(i, x - 1)
	local n = tonumber(s)
	if not n then
		decode_error(str, i, "invalid number '" .. s .. "'")
	end
	return n, x
end


local function parse_literal(str, i)
	local x = next_char(str, i, delim_chars)
	local word = str:sub(i, x - 1)
	if not literals[word] then
		decode_error(str, i, "invalid literal '" .. word .. "'")
	end
	return literal_map[word], x
end


local function parse_array(str, i)
	local res = {}
	local n = 1
	i = i + 1
	while 1 do
		local x
		i = next_char(str, i, space_chars, true)
		-- Empty / end of array?
		if str:sub(i, i) == "]" then
			i = i + 1
			break
		end
		-- Read token
		x, i = parse(str, i)
		res[n] = x
		n = n + 1
		-- Next token
		i = next_char(str, i, space_chars, true)
		local chr = str:sub(i, i)
		i = i + 1
		if chr == "]" then break end
		if chr ~= "," then decode_error(str, i, "expected ']' or ','") end
	end
	return res, i
end


local function parse_object(str, i)
	local res = {}
	i = i + 1
	while 1 do
		local key, val
		i = next_char(str, i, space_chars, true)
		-- Empty / end of object?
		if str:sub(i, i) == "}" then
			i = i + 1
			break
		end
		-- Read key
		if str:sub(i, i) ~= '"' then
			decode_error(str, i, "expected string for key")
		end
		key, i = parse(str, i)
		-- Read ':' delimiter
		i = next_char(str, i, space_chars, true)
		if str:sub(i, i) ~= ":" then
			decode_error(str, i, "expected ':' after key")
		end
		i = next_char(str, i + 1, space_chars, true)
		-- Read value
		val, i = parse(str, i)
		-- Set
		res[key] = val
		-- Next token
		i = next_char(str, i, space_chars, true)
		local chr = str:sub(i, i)
		i = i + 1
		if chr == "}" then break end
		if chr ~= "," then decode_error(str, i, "expected '}' or ','") end
	end
	return res, i
end


local char_func_map = {
	[ '"' ] = parse_string,
	[ "0" ] = parse_number,
	[ "1" ] = parse_number,
	[ "2" ] = parse_number,
	[ "3" ] = parse_number,
	[ "4" ] = parse_number,
	[ "5" ] = parse_number,
	[ "6" ] = parse_number,
	[ "7" ] = parse_number,
	[ "8" ] = parse_number,
	[ "9" ] = parse_number,
	[ "-" ] = parse_number,
	[ "t" ] = parse_literal,
	[ "f" ] = parse_literal,
	[ "n" ] = parse_literal,
	[ "[" ] = parse_array,
	[ "{" ] = parse_object,
}


parse = function(str, idx)
	local chr = str:sub(idx, idx)
	local f = char_func_map[chr]
	if f then
		return f(str, idx)
	end
	decode_error(str, idx, "unexpected character '" .. chr .. "'")
end


function json.decode(str)
	if type(str) ~= "string" then
		error("expected argument of type string, got " .. type(str))
	end
	local res, idx = parse(str, next_char(str, 1, space_chars, true))
	idx = next_char(str, idx, space_chars, true)
	if idx <= #str then
		decode_error(str, idx, "trailing garbage")
	end
	return res
end

-- this is how the main engineer process is passing bullshit to this crap lmao
-- help me.
local fuckjson = os.getenv("ENGINEER_TEMP_PROJ")
local fuck = json.decode(fuckjson)
-- eng.util.print_table(fuck)

-- i appreciate c++
local compiler = ""
if eng.util.endswith(fuck.src, ".c") then
	compiler = fuck.cc
elseif eng.util.endswith(fuck.src, ".cpp") or eng.util.endswith(fuck.src, ".cc") or
eng.util.endswith(fuck.src, ".cxx") or eng.util.endswith(fuck.src, ".c++") then
	compiler = fuck.cxx
else
	print(eng.CONSOLE_COLOR_WARN.."unexpected extension in "..fuck.src..", using "..fuck.cc..eng.CONSOLE_COLOR_RESET)
	compiler = fuck.cc
end

local obj = fuck.proj.builddir.."/obj/"..fuck.src:gsub("/", "@")..".o"

if fuck.proj.type == "sharedlib" then
	fuck.proj.cflags = fuck.proj.cflags.." -fPIC"
end

-- compile frfrfr ong no cap ngl tbh
local cmd = compiler..fuck.proj.cflags.." -o "..obj.." -c "..fuck.src
if fuck.show_command then
	print(cmd)
end
local success = os.execute(cmd)
if not success then
	print(eng.CONSOLE_COLOR_ERROR.."compiling "..fuck.src.." failed"..eng.CONSOLE_COLOR_RESET)

	local file = io.open(fuck.proj.builddir.."/.fail", "w")
	assert(file)
	file:write(":(")
	file:close()
end
