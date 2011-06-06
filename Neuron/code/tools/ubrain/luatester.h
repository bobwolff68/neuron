#ifndef _LUATESTER_H_
#define _LUATESTER_H_
//!
//! \file luatester.h
//! \brief Beginnings of test support infrastructure to allow lua to control existing class frameworks.
//!
//! \author Robert Wolff
//! \date May 23, 2011
//!

#include <iostream>

using namespace std;

class MinibrainTestInstance;

#include "neuroncommon.h"

#include "controller.h"
#include "ubrainmanager.h"

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

extern int luaopen_LuaInterface(lua_State* L);  // declaration of auto-generated entry point

}

#include "oolua.h"
#include "neuronluascript.h"

#define LUACALL(arg, txt) (MinibrainTestInstance::getInstance()->MakeLuaCallback(__FUNCTION__, arg, txt))

//!
//! \class MinibrainTestInstance
//!
//! \brief Replace with one-liner description of the class purpose
//!
//! Details: Replace here for a much more full and complete description of
//!   both the purpose as well as the expectations, usage, etc at a high level.
//!
//! \todo This item needs attention or implemented altogether.
//!       It should operate this way and not that way. See to it soon?
//! \todo Here is another to do item as well.
//!

//!
//!
class MinibrainTestInstance
{
public:
  static MinibrainTestInstance* getInstance()
  {
    static MinibrainTestInstance singleton;
    return &singleton;
  }

  //static uBrainManager*
//!
//! \brief Function used by Lua to set its callback (or disable it).
//!
//! \note Callback has a particular definition / parameter list. This is not documented here
//!	as this is sure to change over time. See ForceCallback/LuaCallback member(s).
//!
//! \param [in] callbackName - a const char pointer to the function name to be called or NULL to disable callbacks.
//! \return 
//!   - true is a success.
//!   - false if there was an error (currently no way to get an error)
//!  
bool SetLuaCallback(const char* callbackName)
  {
    callback = callbackName;
    return true;
  };

  lua_State* getLuaState(void) { return luaScript; };
  
  void PrintHello(void) { cout << endl << "C++::MinibrainTestInstance::HelloWorld - Hi y'all." << endl << endl; };
  bool MakeLuaCallback(const char* fn_caller, int arg, const char* strArg);
  bool RunScript(const char* script);
  
  void setMiniBrain(uBrainManager* mBrain) { miniBrain = mBrain; };
  uBrainManager* getMiniBrain(void) { return miniBrain; };
  
  void setController(Controller* ctrl) { controller = ctrl; };
  Controller* getController(void) { return controller; };
  
// Other non-static member functions
private:
  MinibrainTestInstance() {
    miniBrain = NULL;
    controller = NULL;
    assert(getLuaState());
    luaopen_base(getLuaState());
    luaL_openlibs(getLuaState());
    luaopen_LuaInterface(getLuaState()); 

  };                                 // Private constructor
  
  MinibrainTestInstance(const MinibrainTestInstance&);                 // Prevent copy-construction
  MinibrainTestInstance& operator=(const MinibrainTestInstance&);      // Prevent assignment
  
  string callback;
  NeuronLuaScript luaScript;
  uBrainManager* miniBrain;
  Controller* controller;
};
#endif

