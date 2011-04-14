//============================================================================
// Name        : LuaQuickProof.cpp
// Author      : Bob Wolff
// Version     :
// Copyright   : Copyright 2011 XVD Technology Holdings LTD USA
// Description : Hello World in C++
//============================================================================

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

extern int luaopen_LuaInterface(lua_State* L);  // declaration of auto-generated entry point
}

#include <iostream>
using namespace std;

#include "TestObject.h"
#include "TestingSamples.h"

extern int RunTests(void);

int main(int argc, char** argv)
{
  lua_State *L;

  if (argc<2)
    {
      cout << "Usage: " << argv[0] << " <LuaScriptFile.lua>" << endl;
      return -1;
    }

  if (string(argv[1])=="-test")
    {
      cout << "Running cppunit tests...." << endl;

      return RunTests();
    }
  cout << "Lua quick proof example...Loading Lua..." << endl;

  L = lua_open();
  luaopen_base(L);
  luaL_openlibs(L);
  luaopen_LuaInterface(L);

  if (luaL_loadfile(L, argv[1])==0)  // Load and run file
    {
    if (lua_pcall(L,0,0,0))
      cout << "Error from pcall of script -- " << lua_tostring(L, -1) << endl;
    }
  else
    {
    cout << "Unable to load " << argv[1] << endl;
    cout << "Error from lua load -- " << lua_tostring(L, -1) << endl;
    }
  lua_close(L);

  cout << endl << "Lua unloaded. Exiting Successfully." << endl << endl;
  return 0;
}

int printvalueplusone(int v)
{
  cout << "Value++ : " << v+1 << endl;
  return v+1;
}
