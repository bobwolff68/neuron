/* First try at C and C++ bindings to Lua */
/* USES SWIG to generate the class bindings file. */

%module LuaInterface
%{

extern int printvalueplusone(int value);
#include "TestObject.h"

%}

int printvalueplusone(int value);

%include "TestObject.h"