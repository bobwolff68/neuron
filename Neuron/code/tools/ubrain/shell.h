/*
 * shell.h
 *
 *  Created on: Nov 9, 2010
 *      Author: rwolff
 */

#ifndef SHELL_H_
#define SHELL_H_

#include "neuroncommon.h"
#include "sshmgmt.h"

#include <sstream>
#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>


class Shell
{
public:
	Shell();
	virtual ~Shell();
	void parse(istream& input);
protected:
	bool parseAttributes(stringstream& input);
	int upper(int c) { return std::toupper((unsigned char)c); }
	void strtoupper(string &s);
	bool processCommand(string& cmd, string& subcmd);
	bool processLocal(string& cmd, string& subcmd);
	bool addToXML(string& cmd, string& subcmd);
private:
	SSHManagement ssh;
	typedef map<string,string> attrNameValuePairs;
	attrNameValuePairs namevalues;

};

#endif /* SHELL_H_ */
