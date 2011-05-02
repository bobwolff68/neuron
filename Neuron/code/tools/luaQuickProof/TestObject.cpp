/*
 * TestObject.cpp
 *
 *  Created on: Apr 8, 2011
 *      Author: rwolff
 */

#include <iostream>
using namespace std;

#include "TestObject.h"

TestObject::TestObject()
{
  value = 100;
}

TestObject::~TestObject()
{
  // TODO Auto-generated destructor stub
}

int TestObject::add_n(int n)
{
  value = value + n;

  return value;
}

int TestObject::subtract_n(int n)
{
  value = value - n;
  return value;
}

int TestObject::printvalue()
{
  std::cout << "c++ says: Value is currently " << value << endl;
  return value;
}




