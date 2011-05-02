//============================================================================
// Name        : main_simple.cpp
// Author      : Bob Wolff
// Version     :
// Copyright   : Copyright 2011 XVD Technology Holdings LTD USA
// Description : Simple example of what is required to call 'RunTests()'
//============================================================================

#include <iostream>
using namespace std;

// ***********
// Must 'extern' this function to allow calling it.
// ***********
extern int RunTests(void);

int main(int argc, char** argv)
{
  if (argc<2)
    {
      cout << "Usage: " << argv[0] << " [<other_usage> | -test]" << endl;
      return -1;
    }

  if (string(argv[1])=="-test")
    {
      cout << "Running cppunit tests...." << endl;

      return RunTests();
    }

  cout << "Not testing mode..." << endl;

  return 0;
}

int printvalueplusone(int v)
{
  cout << "Value++ : " << v+1 << endl;
  return v+1;
}
