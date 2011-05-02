/* First try at C and C++ bindings to Lua */
%module LuaInterface
%{

extern int printvalueplusone(int value);
#include "TestObject.h"

%}

int printvalueplusone(int value);

%include "TestObject.h"