engineer = {
	util = {},
	cc = "",
	cxx = "",
}

function engineer.util.silentexec(command)
	-- TODO this will definitely break at some point
	return os.execute(command .. " > /dev/null 2>&1")
end

function engineer.init()
	-- get compiler
	local cc = os.getenv("CC")
	if cc ~= nil and cc ~= "" then
		engineer.cc = cc
	else
		if engineer.util.silentexec("command -v clang") then
			engineer.cc = "clang"
		elseif engineer.util.silentexec("command -v gcc") then
			engineer.cc = "gcc"
		else
			error("no C compiler found. please install gcc or clang, and make sure it's in the PATH")
		end
	end
	print("compiler: " .. engineer.cc)
end

return engineer
