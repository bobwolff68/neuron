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

