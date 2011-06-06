
print ("Hello there from lua.")

function myLuaCallback(fn_caller, a)
	print ("Lua:: myLuaCallback() -  Called from C function=", fn_caller)
	print ("Lua:: myLuaCallback() -  Called from C with ARG=", a)
	if a=="yes" then
	  return true
	else
	  return false
	end
end

local v = LuaInterface.printvalueplusone(7)
print ("Returned result is: " .. v)

local tobj
tobj = LuaInterface.MinibrainTestInstance_getInstance()

tobj:PrintHello();
tobj:SetLuaCallback("myLuaCallback");

print ("Goodbye from lua.")

