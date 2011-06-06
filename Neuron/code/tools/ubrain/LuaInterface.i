/* First try at C and C++ bindings to Lua */
/* USES SWIG to generate the class bindings file. */

%module Neuron
%{

extern int printvalueplusone(int value);
#include "luatester.h"
#include "testjigsupport.h"
#include "controller.h"
#include "ubrainmanager.h"

%}

int printvalueplusone(int value);

%include "luatester.h"
%include "testjigsupport.h"
%rename(TestJig_NEW_SF_DETECTED) NEW_SF_DETECTED;

%nodefaultctor uBrainManager;

/* class uBrainManager {
public:
  void Test(void);
}; */

%nodefaultctor Controller;

class Controller : public TestJigSupport {
public:
  HookID testid;
  bool Hook(HookID id);
  void ShowHooks(void);
};

