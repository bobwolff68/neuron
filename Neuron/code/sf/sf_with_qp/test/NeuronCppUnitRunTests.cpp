//============================================================================
// Name        : NeuronCppUnitRunTests.cpp
// Author      : Bob Wolff
// Version     :
// Copyright   : Copyright 2011 XVD Technology Holdings LTD USA
// Description : Standard CppUnit call for running the complete suite of registered test fixtures
//============================================================================

#include <iostream>
using namespace std;

#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>

int RunTests(void)
{
  // Event manager for and test controller
  CppUnit::TestResult controller;

  // Add a listener that collects test results and prints dots as the tests run.
  CppUnit::TestResultCollector result;
  controller.addListener(&result);
  CppUnit::BriefTestProgressListener progress;
  controller.addListener(&progress);

  // Add the top suite to the test runner (this adds all REGISTERED test fixtures)
  CppUnit::TestRunner runner;
  runner.addTest( CppUnit::TestFactoryRegistry::getRegistry().makeTest() );
  runner.run( controller );

  // Print test output
  CppUnit::CompilerOutputter outputter( &result, CppUnit::stdCOut() );
  outputter.write();

  return result.wasSuccessful() ? 0 : 1;
}
