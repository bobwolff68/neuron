#ifndef _TESTJIGSUPPORT_H_
#define _TESTJIGSUPPORT_H_
//!
//! \file TestJigSupport.h
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
//! \date Wed 01 Jun 2011 11:25:59 AM 
//! 

#include <iostream>
#include <map>

using namespace std;

enum HookID {
	// Control plane (Controller) related items.
	NEW_SF_DETECTED = 0,
	
	// MiniBrain (uBrainManager) related.
	REGISTRATION_COMPLETING,
	
	// SF items.
	
	// SL items.
};

//!
//! \class TestJigSupport
//! 
//! \brief Intended use is to derive from TestJigSupport and allow users of your class 
//! 	to call "HookStall()" to register their desire to hook.
//! 
//! \todo Add parameter(s) to hookstall and support with criss-cross list of an array 
//! 	of maps based on the enum list.
//! 

class TestJigSupport {
public:
	TestJigSupport() { };
	~TestJigSupport() { };
	bool Hook(HookID id);
	bool UnHook(HookID id);
	inline bool IsHooked(HookID id) { return HookedItems[id]; };
	bool ContinueNow(HookID id, const char* fnName);
protected:
	map<HookID, bool> HookedItems;
	
	virtual bool MakeLuaCallback(const char* fn, int arg, const char* txt) = 0;
};
#endif
