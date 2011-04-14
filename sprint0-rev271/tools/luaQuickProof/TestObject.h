/*
 * TestObject.h
 *
 *  Created on: Apr 8, 2011
 *      Author: rwolff
 */

#ifndef TESTOBJECT_H_
#define TESTOBJECT_H_

class TestObject
{
public:
  TestObject();
  virtual
  ~TestObject();
  int value;
  int add_n(int n);
  int subtract_n(int n);
  int printvalue();
  int getvalue() { return value; }
};

#endif /* TESTOBJECT_H_ */
