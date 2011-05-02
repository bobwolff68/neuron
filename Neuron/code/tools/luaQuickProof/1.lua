
function luasimple() 
	print "Inside Lua:: luasimple() -  Called from C. "
end

function luasinglearg(a)
	print ("Inside Lua:: luasinglearg() -  Called from C with ARG=", a)
end

--[[
function luasingleargwithreturn(a,b)
	print ("Inside Lua:: luasingleargwithreturn() -  Called from C with ARG=", a)
	print ("Inside Lua:: luasingleargwithreturn() -  Called from C with 2nd-ARG=", b)
	print ('Changing ret to 5 and attempting to return...')
	b = 5
	return b
end
--]]

function luasingleargwithreturn(a,ret)
	print ('Inside Lua:: luasingleargwithreturn() -  Called from C with ARG=', a)
	print ('Inside Lua:: luasingleargwithreturn() -  and 2nd-arg-ret=', ret)
	print ('Changing ret to 5 and attempting to return...')
	ret = 5
	return 5
end



print ("Hello there from lua.")

local v = LuaInterface.printvalueplusone(7)
print ("Returned result is:")
print (v)

local tobj
tobj = LuaInterface.TestObject()

tobj:printvalue()

print ("Adding 5 to initial value.")
val = tobj:add_n(5)
tobj:printvalue()
print ("Lua return from add_n was: ", val)

print ("Subtracting 10 from value.")
val = tobj:subtract_n(10)
tobj:printvalue()
print ("Lua return from subtract_n was: ", val)

print ("Lua says direct-access to 'value' is: ", tobj.value)

print ("Lua: setting tobj.value arbitrarily from the outside to 999.")
tobj.value = 999
tobj:printvalue()

print ("Goodbye from lua.")

