//!
//! \file luatester.h
//! \brief Beginnings of test support infrastructure to allow lua to control existing class frameworks.
//!
//! \author Robert Wolff
//! \date May 23, 2011
//!

#include <iostream>

using namespace std;

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

extern int luaopen_LuaInterface(lua_State* L);  // declaration of auto-generated entry point

}

#include "oolua.h"
#include "neuronluascript.h"

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

  bool SetLuaCallback(const char* callbackName)
  {
    if (!callbackName) return false;
    if (strlen(callbackName) > 63) return false;
    callback = callbackName;
    return true;
  };

  lua_State* getLuaState(void) { return luaScript; };
  
  void PrintHello(void) { cout << endl << "C++::MinibrainTestInstance::HelloWorld - Hi y'all." << endl << endl; };
  bool ForceCallback(const char* strArg);
  bool RunScript(const char* script);
  bool CallLua(const char* functionname);
  
// Other non-static member functions
private:
  MinibrainTestInstance() {
    assert(getLuaState());
    luaopen_base(getLuaState());
    luaL_openlibs(getLuaState());
    luaopen_LuaInterface(getLuaState()); 

  };                                 // Private constructor
  
  MinibrainTestInstance(const MinibrainTestInstance&);                 // Prevent copy-construction
  MinibrainTestInstance& operator=(const MinibrainTestInstance&);      // Prevent assignment
  
  string callback;
  NeuronLuaScript luaScript;
};

