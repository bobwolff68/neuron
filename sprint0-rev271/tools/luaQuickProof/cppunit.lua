
function luasimple() 
	print "Inside Lua:: luasimple() -  Called from C. "
end

function returnint(a)
	print ('Inside Lua:: returnint() -  Called from C with ARG=', a)
	print ('Inside Lua:: returning a 99...')
	return 99
end

function returnbool(inbool)
	print ('Inside Lua:: returnbool() -  Called from C with ARG=')
	if inbool then print "true" else print "false" end
	return inbool
end

function returnstringpluswow(instr)
	print ('Inside Lua:: returnstring() -  Called from C with ARG=', instr)
	return instr .. " wow"
end

print ("Inside Lua:: Hello there from lua. Functions should be defined now.")

