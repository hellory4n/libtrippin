eng = require("libengineer")
eng.init()

eng.recipe("build", function()
	print("Sigma")
end)

eng.option("sigma", function(val)
	print("sigma on th e wal :" .. val)
end)

eng.run()
