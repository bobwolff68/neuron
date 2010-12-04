/*
 * xmlinout.cpp
 *
 *  Created on: Nov 30, 2010
 *      Author: rwolff
 */

#include "xmlinout.h"

XMLInOut::XMLInOut()
{
    writer = NULL;
    buf = NULL;

    SetupDoc();
}

XMLInOut::~XMLInOut()
{
	DeleteDoc();
}

void XMLInOut::DeleteDoc(void)
{
	if (writer)
	    xmlFreeTextWriter(writer);
	if (buf)
	    xmlBufferFree(buf);

	writer = NULL;
	buf = NULL;

	sections.clear();
}

bool XMLInOut::SetupDoc(void)
{
	DeleteDoc();

    /* Create a new XML buffer, to which the XML document will be
     * written */

    buf = xmlBufferCreate();
    if (buf == NULL) {
        cout << "testXmlwriterMemory: Error creating the xml buffer\n" << endl;
        return false;
    }

    /* Create a new XmlWriter for memory, with no compression.
     * Remark: there is no compression for this kind of xmlTextWriter */
    writer = xmlNewTextWriterMemory(buf, 0);
    if (writer == NULL) {
        cout << "testXmlwriterMemory: Error creating the xml writer\n" << endl;
        return false;
    }

    if (xmlTextWriterSetIndent(writer, 1) < 0)
    	return false;

    /* Start the document with the xml default for the version,
     * encoding ISO 8859-1 and the default for the standalone
     * declaration. */
/*
    rc = xmlTextWriterStartDocument(writer, NULL, "ISO-8859-1", NULL);
    if (rc < 0) {
        printf
            ("testXmlwriterMemory: Error at xmlTextWriterStartDocument\n");
        return false;
    }
*/

    if (AddElement("neuron-brain") < 0)
    	return false;

    if (AddElement("MACROGROUP") < 0)
    	return false;

    // Important addition. When doing a MarkSection(), the # bytes wont be valid for what your
    // intuition tells you. The above <MACROGROUP> at this stage isn't closed due to possible
    // content and attributes. The comment forces things to "end" making the buffer ok for marking.
    if (AddComment("The Real Group of commands starts here.") < 0)
    	return false;

    MarkSection();

    return true;
}

bool XMLInOut::AddCommand(string& cmd, string& subcmd, attrNameValuePairs& namevalues)
{
	if (AddElement(cmd.c_str()) < 0)
		return false;

	if (AddElement(subcmd.c_str()) < 0)
		return false;

    /* Add the set of attributes */
    if (!namevalues.empty())
    {
		  attrNameValuePairs::iterator it;

		  for ( it=namevalues.begin() ; it != namevalues.end(); it++ )
		  {
//		    cout << (*it).first << " => " << (*it).second << endl;

			if (AddAttribute(it->first.c_str(), it->second.c_str())<0)
				return false;
		  }
    }

    // Sub Command
    if (EndElement()<0)
    	return false;

    // Command
    if (EndElement()<0)
    	return false;

    // Now place a marker
    MarkSection();
}

int XMLInOut::AddElement(const char* element)
{
	int rc;

    rc = xmlTextWriterStartElement(writer, BAD_CAST element);
    if (rc < 0)
        cout << "testXmlwriterMemory: Error at element '" << element << "'" << endl;

	return rc;
}

int XMLInOut::EndElement(void)
{
	int rc;

    rc = xmlTextWriterEndElement(writer);
    if (rc < 0)
        cout << "testXmlwriterMemory: Error ending element" << endl;

    return rc;
}

int XMLInOut::AddAttribute(const char* name, const char* value)
{
	int rc;

    rc = xmlTextWriterWriteAttribute(writer, BAD_CAST name,
                                     BAD_CAST value);
    if (rc < 0)
        cout << "testXmlwriterMemory: Error at xmlTextWriterWriteAttribute for '" << name << "'='" << value << "'" << endl;

    return rc;
}

int XMLInOut::AddComment(const char* comment)
{
	int rc;

    rc = xmlTextWriterWriteComment(writer, BAD_CAST comment);
    if (rc < 0)
        cout << "testXmlwriterMemory: Error at xmlTextWriterWriteComment for '" << comment << "'" << endl;

    return rc;
}

bool XMLInOut::SaveScript(const char* filename)
{
	int rc;
	ofstream out;
	// First check to see that we can open the filename before saving/killing the document.

	assert(filename);
	out.open(filename);

	if (out.fail())
		return false;

	rc = xmlTextWriterEndDocument(writer);
    if (rc < 0) {
        printf("testXmlwriterMemory: Error at xmlTextWriterEndDocument\n");
        return false;
    }

    // Now print it or write it...
    cout << "XML Command being saved:" << endl
    		<< "'" << (const char *) buf->content << "'" << endl;

	out << buf->content;
	out.close();

    // Reset back to zero. New document.
    SetupDoc();
    return true;
}

void XMLInOut::MarkSection(void)
{
	int numbytes;

	xmlTextWriterFlush(writer);
	numbytes = xmlBufferLength(buf);
	sections.push_front(numbytes);

//	cout << "Current marked buffer @ index-end:" << numbytes << endl
//			<< "'" << (const char*)buf->content << "'" << endl;
}

bool XMLInOut::GetGroup(string& str_out, int n)
{
	int bytes_offset = 0;
	int length;

	if (n+1 > sections.size())
	{
		str_out = "ERROR - not enough sections.";
		return false;
	}

	// If we are looking for the 'last' entry (the beginning of the document, bytes_offset is zero.
	// Otherwise we calculate it here by looking beyond the item of interest by one as that's the beginning of the item of interest.
	if (n+1 < sections.size())
		bytes_offset = sections[n+1];

	length = sections[n] - bytes_offset;

//	if (bytes_offset)
//		length++;

	str_out = (const char *) &buf->content[bytes_offset];
	// Now lop off the tail by keeping the left-most 'length'
	str_out = str_out.substr(0, length);

	return true;
}
