trippin = {}

function trippin.init()
	print("hi :)")
end

function trippin.rule(name, callback)
	if arg[1] == name then
		callback()
	end
end

return trippin