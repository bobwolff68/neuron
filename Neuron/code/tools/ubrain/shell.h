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
#include "localitems.h"

#include <sstream>
#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>


class Shell
{
public:
	Shell();
	virtual ~Shell();
	void parse(istream& input);
	LocalItems local;
protected:
	bool parseAttributes(const char* inputstr);
	int upper(int c) { return std::toupper((unsigned char)c); }
	void strtoupper(string &s);
	bool processCommand(string& cmd, string& subcmd);
	bool processLocal(string& cmd, string& subcmd);
	bool addToXML(string& cmd, string& subcmd);
	bool requiredAttributesPresent(string& subcmd, const char* attr1, const char* attr2="", const char* attr3="", const char* attr4="");
private:
	SSHManagement ssh;
	typedef map<string,string> attrNameValuePairs;
	attrNameValuePairs namevalues;

};

#endif /* SHELL_H_ */
