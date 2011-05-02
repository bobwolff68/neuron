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

// ***************
// Makes sure this class/testfixture gets included in the global suite.
// ***************
CPPUNIT_TEST_SUITE_REGISTRATION( TestingSamples );

// ***************
// Now author your individual tests which were defined in the class definition
// ***************
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
