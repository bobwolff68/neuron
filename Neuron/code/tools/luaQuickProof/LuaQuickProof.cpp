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

#include "oolua.h"

extern int RunTests(void);

void CallLua(OOLUA::Script& s)
{
//s.call.bind_script(L);
//OOLUA::setup_user_lua_state(L);
	cout << "=========================================" << endl;
	cout << "CallLua() - Starting to try C->Lua calls." << endl;
/*
	s.run_chunk("function luasimple() \n"
		"print 'Inside Lua:: luasimple() -  Called from C. '\n"
		"end\n\n"
		"function luasinglearg(a)\n"
		"print ('Inside Lua:: luasinglearg() -  Called from C with ARG=', a)\n"
		"end\n"
		"function luasingleargwithreturn(a,ret)\n"
		"print ('Inside Lua:: luasingleargwithreturn() -  Called from C with ARG=', a)\n"
		"print ('Inside Lua:: luasingleargwithreturn() -  and 2nd-arg-ret=', ret)\n"
		"print ('Changing ret to 5 and attempting to return...')\n"
		"ret = 5\n"
		"return 5\n"
		"end");
*/
// Test #1
	s.call("luasimple");

// Test #2 - single parameter - no return.
	int a=100;
	s.call("luasinglearg",a);

// Test #3 - single parameter - with return. return is passed in (by reference?)
	int b=200;
	int ret=40;
	int real_ret=-5;
	s.call("luasingleargwithreturn",b,ret);
	OOLUA::pull2cpp(s,real_ret);
	cout << "Back in C...returned real_ret=" << real_ret << " while ret=" << ret << endl;

	cout << "CallLua() - Finished with C->Lua calls." << endl;
	cout << "=========================================" << endl;
}

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

OOLUA::Script s1;
//  L = lua_open();
  L = s1;

  luaopen_base(L);
  luaL_openlibs(L);
  luaopen_LuaInterface(L); 

  if (! s1.run_file(argv[1]))  // Load and run file
    {
    cout << "Unable to load or run " << argv[1] << endl;
    cout << "Error from lua load -- " << OOLUA::get_last_error(L) << endl;
    }

  // Now try calling down to Lua from C/C++
  CallLua(s1);

  cout << endl << "Lua unloaded. Exiting Successfully." << endl << endl;
  return 0;
}

int printvalueplusone(int v)
{
  cout << "Value++ : " << v+1 << endl;
  return v+1;
}
