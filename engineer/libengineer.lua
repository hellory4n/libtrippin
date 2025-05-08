--[[
	Engineer v0.1.0

	Bestest build system ever
	More information at https://github.com/hellory4n/libtrippin/blob/engineer/README.md

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
	-- Table of strings with functions (no arguments, no returns)
	options = {},

	-- constants
	CONSOLE_COLOR_RESET = "\27[0m",
	CONSOLE_COLOR_LIB = "\27[0;90m",
	CONSOLE_COLOR_WARN = "\27[0;93m",
	CONSOLE_COLOR_ERROR = "\27[0;91m",
}

-- Runs a command with no output
function eng.util.silentexec(command)
	-- TODO this will definitely break at some point
	return os.execute(command .. " > /dev/null 2>&1")
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
                    output_str = output_str .. ",\n"
                elseif not (string.find(output_str,"\n",output_str:len())) then
                    output_str = output_str .. "\n"
                end

                -- This is necessary for working with HUGE tables otherwise we run out of memory using concat on huge strings
                table.insert(output,output_str)
                output_str = ""

                local key
                if (type(k) == "number" or type(k) == "boolean") then
                    key = "["..tostring(k).."]"
                else
                    key = "['"..tostring(k).."']"
                end

                if (type(v) == "number" or type(v) == "boolean") then
                    output_str = output_str .. string.rep('\t',depth) .. key .. " = "..tostring(v)
                elseif (type(v) == "table") then
                    output_str = output_str .. string.rep('\t',depth) .. key .. " = {\n"
                    table.insert(stack,node)
                    table.insert(stack,v)
                    cache[node] = cur_index+1
                    break
                else
                    output_str = output_str .. string.rep('\t',depth) .. eng.CONSOLE_COLOR_RESET .. key .. eng.CONSOLE_COLOR_RESET .. " = '"..tostring(v).."'"
                end

                if (cur_index == size) then
                    output_str = output_str .. "\n" .. string.rep('\t',depth-1) .. "}"
                else
                    output_str = output_str .. ","
                end
            else
                -- close the table
                if (cur_index == size) then
                    output_str = output_str .. "\n" .. string.rep('\t',depth-1) .. "}"
                end
            end

            cur_index = cur_index + 1
        end

        if (size == 0) then
            output_str = output_str .. "\n" .. string.rep('\t',depth-1) .. "}"
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

-- Initializes engineer™
function eng.init()
	print(eng.CONSOLE_COLOR_LIB .. "Engineer v0.1.0" .. eng.CONSOLE_COLOR_RESET)

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
	print(eng.CONSOLE_COLOR_LIB .. "C compiler: " .. eng.cc .. eng.CONSOLE_COLOR_RESET)

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
	print(eng.CONSOLE_COLOR_LIB .. "C++ compiler: " .. eng.cxx .. eng.CONSOLE_COLOR_RESET)

	-- TODO default "help" recipe
end

-- Makes a recipe. The callback will be called when the recipe is used.
function eng.recipe(name, callback)
	eng.recipes[name] = callback
end

-- Adds an option. The callback takes in whatever value the option has (string), and is only called
-- if that option is used.
function eng.option(name, callback)
	eng.options[name] = callback
end

-- Put this at the end of your build script so it actually does something
function eng.run()
	opts = {}
	recipes = {}

	for _, argma in ipairs(arg) do
		local key, val = argma:match("^(%w+)=?(%S*)$")
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
			print(eng.CONSOLE_COLOR_WARN .. "unknown option \"" .. mate.key .. "\"" .. eng.CONSOLE_COLOR_RESET)
		end
	end

	for _, recipema in ipairs(recipes) do
		if eng.recipes[recipema] ~= nil then
			eng.recipes[recipema]()
		else
			print(eng.CONSOLE_COLOR_WARN .. "unknown recipe \"" .. recipema .. "\"" .. eng.CONSOLE_COLOR_RESET)
		end
	end
end

return eng
