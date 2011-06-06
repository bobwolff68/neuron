//!
//! \file testjigsupport.cpp
//! 
//! \brief This class is intended to be a derived-from class for any classes which are 
//! 	going to be controlled or impededed or modified by Lua's access to other classes
//!	such as MiniBrainTestInstance. There are a growing number of enums which identify
//!	the type of hooks to make. This allows the parent to check to see if something
//!	is currently hooked so it can take an appropriate action.
//!
//! \note It is imperative that this class be kept lean and mean as it will be utilized
//! 	for these 'checks' for hooks right inline in the code.
//! 
//! \author rwolff
//! \date Wed 01 Jun 2011 12:20:57 PM 
//! 

#include <assert.h>
#include "testjigsupport.h"

bool TestJigSupport::Hook(HookID id)
{
	if (HookedItems[id])
		return false;
	
	HookedItems[id] = true;
	return true;
}

bool TestJigSupport::UnHook(HookID id)
{
	if (!HookedItems[id])
		return false;
		
	HookedItems.erase(id);
	return true;
}

bool TestJigSupport::ContinueNow(HookID id, const char* fnName)
{
//TODO: Here is where we look at the hooked "specific items" list 
//	(for SF this would be a list of SFs which are hooked instead of ALL currently.

	assert(fnName);

	MakeLuaCallback(fnName, (int)id, "junk");
	
}

