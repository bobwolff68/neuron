#ifndef _TESTJIGSUPPORT_H_
#define _TESTJIGSUPPORT_H_
//!
//! \file TestJigSupport.h
//! 
//! \author rwolff
//! \date Wed 01 Jun 2011 11:25:59 AM 
//! 

#include <iostream>
#include <map>

using namespace std;

//!
//! The HookID enum is where we create uniquely named ID names for those items we wish to
//! use as hooks in an existing class. This Hook() allows the running code to check in 
//! strategic locations for the usage of the hook and if it is there, then the running
//! program can give a call down to Lua with this id in order to get a "go/continue" or
//! "no-go/stall". This bool return can be used for other specific behaviors. It just
//! needs to be documented very well what is intended and expected by both sides in
//! the running C++ class as well as in Lua.
//! 
enum HookID {
	// Control plane (Controller) related items.
	NEW_SF_DETECTED = 10,
	
	// MiniBrain (uBrainManager) related.
	REGISTRATION_COMPLETING,
	
	// ACP Slave items.
	EXAMPLE_WAIT_FOR_DISCOVERY,
	EXAMPLE_WAIT_FOR_READY,
	
	// SL items.
};

//!
//! \class TestJigSupport
//! 
//! \brief This class is intended to be a derived-from class for any classes which are 
//! 	going to be controlled or impededed or modified by Lua's access to other classes
//!	such as MiniBrainTestInstance. There are a growing number of enums which identify
//!	the type of hooks to make. This allows the derived class to check to see if something
//!	is currently hooked so it can take an appropriate action.
//!
//!	As an over-arching principle, a) take a class which needs mofied behavior possibilities
//!	via Lua and derive it from TestJigSupport. b) Make a new ID or many new IDs in HookID for
//!	your purposes. c) Flesh out MakeLuaCallback() in your class. This will likely be:
//! \code class Mine : TestJigSupport {
//!		int a;
//!		....[functions etc]
//!		bool MakeLuaCallback(const char* fn, int arg, const char* txt) {
//!			return MinibrainTestInstance::getInstance()->MakeLuaCallback(fn, arg, txt); };
//!		.... }
//! \endcode
//! \note It is imperative that this class be kept lean and mean as it will be utilized
//! 	for these 'checks' for hooks right inline in the code.
//! 
//! \todo Add parameter(s) to hookstall and support with criss-cross list of an array 
//! 	of maps based on the enum list.
//! 

class TestJigSupport {
public:
	TestJigSupport() { };
	~TestJigSupport() { };
//!
//! Hook
//! 
//! \brief This is the starting function which is generally called by Lua to enable some kind of
//!		runtime testing or behavior modification situation.
//! 
//! \note 'id' must be one of the pre-defined enum IDs in HookID. Hook() returns false if there was a problem.
//! 		One problem would be if this id given is already Hooked.
//! 
//! \param HookID id
//! \return bool
//! 
//! 
	bool Hook(HookID id);
//!
//! UnHook
//! 
//! \brief This is how Lua (or C++) can release the hook on a particular ID. This will essentially defeat the hook.
//! 
//! \note 'id' must be one of the pre-defined enum IDs in HookID. Hook() returns false if there was a problem.
//! 		One problem here would be if the id is not currently Hooked, then it cannot be UnHooked.
//! 
//! \param HookID id
//! \return bool
//! 
//! 
	bool UnHook(HookID id);
//!
//! IsHooked
//! 
//! \brief Simple method to check whether an id is actively hooked or not.
//! 
//! \access inline
//! \param HookID id
//! \return bool
//! 
	inline bool IsHooked(HookID id) { return HookedItems[id]; };
//!
//! ContinueNow
//! 
//! \brief Running C++ code should call ContinueNow() when they have recognized 'id' is Hooked via IsHooked(id) and
//! 		they are looking to validate if Lua is going to modify the course of behavior.
//! 
//! \param HookID id
//! \param const char* fnName
//! \return bool
//! 
//! 
	bool ContinueNow(HookID id, const char* fnName);
	//! Print what hooks are enabled at this time.
	void ShowHooks(void);
	HookID testid;
protected:

	//! Pretty simple lookup map which allows us to store a hook ID and pair it with true or false.
	//! This makes it quite efficient despite low or high numbers of hooks in use.
	map<HookID, bool> HookedItems;

	//!
	//! \brief Pure Virtual - this function is used as the method by which specific functions in Lua are called
	//! by the running C++ program. This must be implemented by the derived class. It is typical to see:
	//! \code bool MakeLuaCallback(const char* fn, int arg, const char* txt) {
	//! 	return MinibrainTestInstance::getInstance()->MakeLuaCallback(fn, arg, txt); 
	//! }; \endcode
	//! 	However, in some classes, the use of MinibrainTestInstance directly has been found to cause some
	//!	compiler issues. So, for now, we have shorted-out the problem in this manner.
	//! 
	virtual bool MakeLuaCallback(const char* fn, int arg, const char* txt) = 0;
};
#endif

