
print()
print ("Lua:: Unit test for New SF stalling.")
print()

--strout = string.format("checking value=%d and then we're done.", 99)
--print (strout)

function MiniBrainCallback(fn_caller, arg, txt)
	print ("Lua:: myLuaCallback()")
	print ("   -  Called from C function=" .. fn_caller)
	print ("   -  Called from C with ARG=", arg)
	print ("   -  Called from C with TXT=" .. txt)

	if fn_caller=="RemoteSFUpdate" then
	  if arg==Neuron.TestJig_NEW_SF_DETECTED then
	  	print ("Lua:: Stalling NEW_SF_DETECTED")
	  	return false
	  else
	  	print ("Lua:: Argument unrecognized:", arg)
	  	return true
	  end
	else
	  print ("Lua:: During callback, fn_caller unknown: " .. fn_caller)
	  return false
	end
end

local tobj
tobj = Neuron.MinibrainTestInstance_getInstance()

tobj:SetLuaCallback("MiniBrainCallback");

ctrlr = tobj:getController()
ctrlr:Hook(TestJig_NEW_SF_DETECTED)


