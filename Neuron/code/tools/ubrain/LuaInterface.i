/* First try at C and C++ bindings to Lua */
/* USES SWIG to generate the class bindings file. */

%module Neuron
%{

extern int printvalueplusone(int value);
#include "luatester.h"
#include "testjigsupport.h"

%}

int printvalueplusone(int value);

%include "luatester.h"
%include "testjigsupport.h"
%rename(TestJig_NEW_SF_DETECTED) NEW_SF_DETECTED;
