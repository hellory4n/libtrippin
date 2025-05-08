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

	-- get c compiler
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
end

return eng
