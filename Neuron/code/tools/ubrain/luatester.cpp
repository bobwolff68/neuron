
#include "luatester.h"
#include <iostream>

using namespace std;


int printvalueplusone(int input)
{
  cout << "C++::printvalueplusone:: input==" << input << " while output should be: " << input+1 << endl;
  return input+1;
}

bool MinibrainTestInstance::MakeLuaCallback(const char* fn_caller, int arg, const char* strArg)
{
  cout << "C++::Executing a callback to Lua ... " << endl;
  
  bool success;
  bool moveOn;
  
  assert(callback != "");
  if (callback == "")
  {
    cout << "Cannot make callback. Use SetLuaCallback(char*) prior to making a callback." << endl;
    return false;
  }
  
  success = luaScript.call(callback, fn_caller, arg, strArg);
  if (!success)
  {
    cout << "C++::Calling Lua callback function " << callback << "() failed." << endl;
    return false;
  }
  
  success = luaScript.ReturnValue(moveOn);
  if (success)		// Callback was good. Now the RESULT of the callback is in 'moveOn'
  {
    cout << "Callback Says: " << (moveOn ? "Move On." : "Do not continue.") << endl;
    return moveOn;
  }
  else
  {
    cout << "C++::Call to Lua callback function did not yield a result (bool) as expected." << endl;
    return false;
  }
    
}

bool MinibrainTestInstance::RunScript(const char* script)
{
  assert(script);

  if (!script)
  {
    cout << "Script is NULL. ERROR." << endl;
    return false;
  }
  
  if (! luaScript.run_file(script))  // Load and run file
    {
      cout << "Unable to load or run " << script << endl;
      cout << "Error from lua load -- " << OOLUA::get_last_error(luaScript) << endl;
      return false;
    }
    
    return true;
}

