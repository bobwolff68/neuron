/*
 * TestingSamples.cpp
 *
 *  Created on: Apr 13, 2011
 *      Author: rwolff
 */

// ***************
// MUST include this in your cppunit implementation file
// ***************
#include "cppunit/config/SourcePrefix.h"

#include "TestingSamples.h"
#include "neuronluascript.h"

using namespace std;

// ***************
// Makes sure this class/testfixture gets included in the global suite.
// ***************
CPPUNIT_TEST_SUITE_REGISTRATION( TestingSamples );

// ***************
// Now author your individual tests which were defined in the class definition
// ***************
void TestingSamples::LuaReturns()
{
  lua_State* L;
  L = lua_open();
  CPPUNIT_ASSERT(L != NULL);

  // Open up s1 with an existing lua_State
  NeuronLuaScript s1(L);
  
  luaopen_base(L);
  luaL_openlibs(L);

  if (s1.run_file("cppunit.lua")==0)  // Load and run file
    {
    cout << "Unable to load or run cppunit.lua" << endl;
    cout << "Error from lua load -- " << lua_tostring(L, -1) << endl;
    }
    
    int value = 100;
    int ret;
    bool success;
    success = s1.call("returnint", value);
    CPPUNIT_ASSERT(success == true);
    
    success = s1.ReturnValue(ret);
    CPPUNIT_ASSERT(success == true);
    CPPUNIT_ASSERT(ret == 99);
    
    // Now test bool return both true and false...
    bool test=true;
    bool retbool;
    success = s1.call("returnbool", test);
    CPPUNIT_ASSERT(success == true);
    
    success = s1.ReturnValue(retbool);
    CPPUNIT_ASSERT(success == true);
    CPPUNIT_ASSERT(retbool == test);
    
    test = false;
    success = s1.call("returnbool", test);
    CPPUNIT_ASSERT(success == true);
    
    success = s1.ReturnValue(retbool);
    CPPUNIT_ASSERT(success == true);
    CPPUNIT_ASSERT(retbool == test);

// Now test string return case.
// Input 'This is something' and get back 'This is something wow'
    std::string teststr("This is something");
    std::string retstr;
    
    success = s1.call("returnstringpluswow", teststr);
    CPPUNIT_ASSERT(success == true);
    
    success = s1.ReturnValue(retstr);
    CPPUNIT_ASSERT(success == true);
    CPPUNIT_ASSERT(retstr == teststr + " wow");
}

void TestingSamples::unit()
{
  // Seriously rudamentary
//  CPPUNIT_ASSERT( 1 == 2 );
  // At least using a class to be tested.
  CPPUNIT_ASSERT( t1->getvalue() == t2->getvalue() );
  t1->add_n(5);
  t1->printvalue();
  CPPUNIT_ASSERT( t1->getvalue() == (t2->getvalue() + 5) );
  // Seriously rudamentary
//  CPPUNIT_ASSERT( 3 == 4 );
}

void TestingSamples::sanity()
{
  // Start with known values.
  t1->value = 0;
  t2->value = 0;

  // Test addition is working
  CPPUNIT_ASSERT( t1->add_n(5) == 5 );
  // Test subtraction gives back expected results from a known starting point.
  CPPUNIT_ASSERT( t2->subtract_n(15) == -15 );
}

void TestingSamples::invalid()
{
  CPPUNIT_ASSERT( 5 == 0 );
}
