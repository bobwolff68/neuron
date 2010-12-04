/*
 * xmlinout.h
 *
 *  Created on: Nov 30, 2010
 *      Author: rwolff
 */

#ifndef XMLINOUT_H_
#define XMLINOUT_H_

#include "neuroncommon.h"

#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>
#include <stack>

class XMLInOut
{
public:
	XMLInOut();
	virtual ~XMLInOut();

	typedef map<string,string> attrNameValuePairs;

	bool AddCommand(string& cmd, string& subcmd, attrNameValuePairs& namevalues);
	bool SaveScript(const char* filename);
	bool GetGroup(string& str_out, int n=0);

protected:
    xmlTextWriterPtr writer;
    xmlBufferPtr buf;
    deque<int> sections;

	bool SetupDoc(void);
	void DeleteDoc(void);
	int AddElement(const char* element);
	int EndElement(void);
	int AddAttribute(const char* name, const char* value);
	int AddComment(const char* comment);

	void MarkSection(void);

};

#endif /* XMLINOUT_H_ */
