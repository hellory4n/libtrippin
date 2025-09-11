#!/bin/lua
local sam = require("samurai/samurai")
sam.init()

sam.option("mycock", "MY COCK NOTATION!", function(val)
	print("MYCOCK " .. val)
end)

sam.run()
