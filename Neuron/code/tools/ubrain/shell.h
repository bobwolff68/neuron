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
#include "xmlinout.h"
#include "ubrainmanager.h"

class Shell
{
public:
	Shell(uBrainManager* pMgr);
	virtual ~Shell();
	bool parseLine(istream& input, bool isScript=false);
	bool ProcessScript(const char* fname);

	XMLInOut xml;

protected:
	bool parseAttributes(const char* inputstr);
	bool processCommand(string& cmd, string& subcmd);
	bool addToXML(string& cmd, string& subcmd);
	bool ProcessSingleCommand(const char* tags);
	void ProcessNode(xmlTextReaderPtr reader);

private:
	typedef map<string,string> attrNameValuePairs;
	attrNameValuePairs namevalues;

	uBrainManager* pBrainManager;

};

#endif /* SHELL_H_ */
