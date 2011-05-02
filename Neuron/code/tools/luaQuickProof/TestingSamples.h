/*
 * TestingSamples.h
 *
 *  Created on: Apr 13, 2011
 *      Author: rwolff
 */

#ifndef TESTINGSAMPLES_H_
#define TESTINGSAMPLES_H_

// ***************
// MUST include this in the header file where you will define your TestFixture
// ***************
#include <cppunit/extensions/HelperMacros.h>

#include "TestObject.h"

// ***************
// Tests are derived from TestFixture
// ***************
class TestingSamples : public CppUnit::TestFixture
{
  // ***************
  // Match argument name to class name
  // ***************
  CPPUNIT_TEST_SUITE( TestingSamples );
  // ***************
  // List of each test function call name
  // ***************
  CPPUNIT_TEST( unit );
  CPPUNIT_TEST( sanity );
  CPPUNIT_TEST( invalid );
  CPPUNIT_TEST( LuaReturns );
  CPPUNIT_TEST_SUITE_END();

public:
  TestingSamples() { };
  virtual
  ~TestingSamples() { };
public:
  // ***************
  // Do all that is necessary to setup and teardown your test environment.
  // ***************
  void setUp() { t1 = new TestObject; t2 = new TestObject; };
  void tearDown() { if (t1) delete t1; if (t2) delete t2; };

protected:
  TestObject *t1, *t2;
  // ***************
  // These function names are not magical. Define the names you want above
  // and list them here. Implement these same functions in your implementation file.
  // ***************
  void unit();
  void sanity();
  void invalid();
  void LuaReturns();

};

#endif /* TESTINGSAMPLES_H_ */
