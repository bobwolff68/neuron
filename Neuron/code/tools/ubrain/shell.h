/*
 * shell.h
 *
 *  Created on: Nov 9, 2010
 *      Author: rwolff
 */

#ifndef SHELL_H_
#define SHELL_H_

#include "neuroncommon.h"
#include <sstream>

class Shell
{
public:
	Shell();
	virtual ~Shell();
	void parse(istream& input);
protected:
	int upper(int c) { return std::toupper((unsigned char)c); }
	void strtoupper(string &s);
	bool processCommand(string& cmd, string& subcmd, string& attributes);

};

#endif /* SHELL_H_ */
