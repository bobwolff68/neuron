
print()
print ("Lua:: Unit test for New SF stalling.")
print()

initialized = false

local mbti
local mbrain
local ctrlr
mbti = Neuron.MinibrainTestInstance_getInstance()

mbti:SetLuaCallback("MiniBrainCallback");

--strout = string.format("checking value=%d and then we're done.", 99)
--print (strout)

function MiniBrainCallback(fn_caller, arg, txt)
	str = string.format("Lua::myLuaCallback() - Caller='%s', Arg=%d, Txt='%s'", fn_caller, arg, txt);
	print (str)

-- Opportunity to Initialize our hooks and get the controller and/or minibrain
	if txt=="INITIALIZE" then
		if not mbti then
			print("Lua::INIT::mbti is nil. Big problem.")
		end
		
		mbrain = mbti:getMiniBrain()
		if not mbrain then
			print("Lua::INIT::No mbrain pointer.")
		end
		
		ctrlr = mbti:getController()
		if ctrlr then
			suc = ctrlr:Hook(Neuron.NEW_SF_DETECTED)
			if not suc then
				print("Lua::INIT::Hook() returned false.")
			end
			print("Lua::INIT::showing hooks list...")
			ctrlr:ShowHooks()
			initialized = true
			print("Lua::INIT complete.")
			return true
		else
			print("Lua::INIT::getController() failed. Likely not setup by C++ side.")
			print("Lua::INIT::-- Make sure to setController() and setMiniBrain() prior to INITIALIZE.")
			return false
		end
		
	end

-- Safety mechanism to halt any inbound callbacks until initialization has taken place.
	if not initialized then
		print("Lua:: ERROR: Callback issued **PRIOR** to making 'INITIALIZE' callback.")
		print("Lua:: Callback will be ignored.")
		return false
	end
	
	if fn_caller=="RemoteSFUpdate" then
	  if arg==Neuron.NEW_SF_DETECTED then
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



