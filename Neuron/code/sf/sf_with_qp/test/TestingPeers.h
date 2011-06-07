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
  CPPUNIT_TEST( TestCase_SF_CreateTheInstance );
  CPPUNIT_TEST( TestCase_SF_DestroyTheInstance );
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
  void TestCase_SF_CreateTheInstance();
  void TestCase_SF_DestroyTheInstance();
};

#endif /* TESTINGSAMPLES_H_ */
