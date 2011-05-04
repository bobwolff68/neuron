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

// ***************
// Tests are derived from TestFixture
// ***************
class TestingPeers : public CppUnit::TestFixture
{
  // ***************
  // Match argument name to class name
  // ***************
  CPPUNIT_TEST_SUITE( TestingPeers );
  // ***************
  // List of each test function call name
  // ***************
  CPPUNIT_TEST( Run_Entity_Info_Discovery_Test );
  CPPUNIT_TEST_SUITE_END();

public:
  TestingPeers() { };
  virtual
  ~TestingPeers() { };
public:
  // ***************
  // Do all that is necessary to setup and teardown your test environment.
  // ***************
  void setUp();
  void tearDown();

protected:
  // ***************
  // These function names are not magical. Define the names you want above
  // and list them here. Implement these same functions in your implementation file.
  // ***************
  void Run_Entity_Info_Discovery_Test();
};

#endif /* TESTINGSAMPLES_H_ */
