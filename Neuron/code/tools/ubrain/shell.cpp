/*
 * shell.cpp
 *
 *  Created on: Nov 9, 2010
 *      Author: rwolff
 */

#include "shell.h"

#include <string.h>

Shell::Shell()
{
	// TODO Auto-generated constructor stub

}

Shell::~Shell()
{
	// TODO Auto-generated destructor stub
}

//
//   T H E  L A N G U A G E
//
// <command> [<subcommand> [<parameters>]]
//
// STATUS
// 		No argument... - Show whether keys are all set. Stun server address, # of endpoints, # of SFs, # of entities
//		FACTORIES	- List of SFs in the system with how many SL/Entities for each
//		ENDPOINTS	- List of endpoints and how many Entites for each
//
// QUIT|EXIT|^D
// - Exit shell parsing and quit.
//
// SET
// 		STUNSERVER <ip_address>[:port]
//      VERBOSITY <level>
//
// SCRIPT <script_to_run_via_parser>
// - Open and read/parse filename - expected to be used for demo macros.
//
// SF
//		CREATE <ip_address>
// 		- ip address where a single SF will be located. ssh is utilized to execute the sf remotely.
//		ADDSESSION <ip_address> <session_id>
//		- Creates a session remotely at <ip_address> of session number <session_id>
//		ADDENTITY <ip_address> <session_id> <entity_type> [args....]
//		- Adds various types of "entity_type" items to a remote SF.
//			<entity_type> can be any of:
//			RP <source_peer> <subscriber_peer>
//			FILESOURCE <name> <subscriber_peer> <file_location>
//			DECODESINK <source_peer>
//
// SETUP
//		CREATEKEYPAIR
//		- Creates a local public/private keypair in ~/.ssh/id_rsa* if one does not exist.
//		SENDPUBLICKEY <ip_address>
//		- Send ~/.ssh/id_rsa.pub to remote <ip_address> to setup password-less authentication.
//		TESTAUTHENTICATION <ip_address>
//		- Quick programmatic test which expects a known formatted output from 'sf --version' remotely
//
// ADD
//		SESSION <session_id>
//		- Internal list for uBrain to maintain of sessions. Can be caused by script or indirectly
//		  by an Endpoint who wants to create a new session/call.
//
void Shell::parse(istream& input)
{
	string line;
	stringstream linestream;
	string cmd, subcmd;
	stringstream attrstream;
	bool eol;

	cout << endl << "uBrain > " << flush;

	while (getline(input, line))
	{
		namevalues.clear();

		if (line=="" || line == "\n" || line == "\r")
		{
			cout << "uBrain > " << flush;
			continue;
		}

//		cout << "line: " << line << endl;

		cmd = "";
		subcmd = "";

		linestream.clear();
		linestream.str(line);
//		cout << "linestream: " << linestream.str() << endl;
		//getline(linestream, word, ' ');
/*		while (!(linestream.rdstate() & ifstream::eofbit))
		{
			linestream >> word;
			cout << "word: " << word << endl;
		}
*/

		//
		// All commands are of the form <cmd> [<subcmd> [attributes]]
		//
		eol = false;
		while (!eol)
		{
			static char attr[100];
			int lentoeol;

			linestream >> cmd;
			// Command with no subcommand nor attributes
			if (linestream.rdstate() & ifstream::eofbit)
				break;

			linestream >> subcmd;
			if (linestream.rdstate() & ifstream::eofbit)
				break;

			lentoeol = linestream.readsome(attr, 99);
			attr[lentoeol] = 0;

			int st=0;
			while (attr[st]==' ')
				st++;

			parseAttributes(&attr[st]);

			eol=true;
		}

//		cout << "Command: " << cmd << " -- SubCommand: " << subcmd << " -- Attributes: " << attributes << endl;

		// Now process.
		if (cmd!="")
			processCommand(cmd, subcmd);

		cout << "uBrain > " << flush;
	}

}

void Shell::strtoupper(string &s)
{
//	std::transform(s.begin(), s.end(), s.begin(), upper);
	  std::string::iterator i = s.begin();
	  std::string::iterator end = s.end();

	  while (i != end) {
	    *i = upper((unsigned char)*i);
	    ++i;
	  }
}

///
/// \brief parseAttributes must take a stringstream and process its name=value pairs. This must
///			also take into consideration spaces before/after the '=' and the use of '"' as delimters on values.
///			Care is taken to parse odd situations, but this is intended to be a well behaved input either from
///			a machine-written xml generator or a careful human inputting line-oriented commands.
///
bool Shell::parseAttributes(const char* inputstr)
{
	string w;
	char buff[100];
	string name;
	string value;
	stringstream input;

	input << inputstr;
	cout << "Input has contents:'" << input.str() << "'." << endl;

	// Erase all from the list.
	namevalues.clear();

	// Indicator that we're at the end.
	do
	{
		// Eat pre-run-whitespace
		while (input.peek()==' ')
			input.get();

		// Get first name (delimited by '=')
		// If not found eofbit will be set and we're in a syntax error state.
		input.getline(buff, 99, '=');

		// If we have a failure here, it may just mean we're in a non-sense attrib list or at the end of a list.
		// Dont fail...just break so that the length of the name/value pair list will determine success.

		ios::iostate st;
		st = input.rdstate();
		if (st & ifstream::eofbit)
			break;

//		cout << "RAW:'" << buff << "'" << endl;

		// Eat all trailing ' ' spaces.
		while (buff[strlen(buff)-1]==' ')
			buff[strlen(buff)-1]=0;
//		cout << "Processed:'" << buff << "'" << endl;

		name = buff;	// Copies buff string into name.

		// Moving on to the next part (the value - either quoted or not)
		// Eat pre-run-whitespace
		while (input.peek()==' ')
			input.get();

//		cout << "NEXTCHAR:'" << s.peek() << "'" << endl;

		// If next char is '"', we must eat all '"' if more than one and then read until '"'
		// else just read till next space.
		if (input.peek()=='\"')
		{
			while (input.peek()=='\"')
			{
				input.get();
				if (input.rdstate() & ifstream::eofbit)
					return false;
			}

			// Now first '"' is read...now read till '"'
			input.getline(buff, 99, '\"');

			// Eat all trailing ' ' spaces.
			while (buff[strlen(buff)-1]==' ')
				buff[strlen(buff)-1]=0;

			value = buff;
		}
		else
			input >> value;

//		cout << "RAW:'" << buff << "'" << endl;

//		cout << "Processed:'" << buff << "'" << endl;

		namevalues[name] = value;
	} 	while (input && !(input.rdstate() & ifstream::eofbit));


	// Now iterate the list and print them.
	if (!namevalues.empty())
	{
		  attrNameValuePairs::iterator it;

		  for ( it=namevalues.begin() ; it != namevalues.end(); it++ )
		    cout << (*it).first << " => " << (*it).second << endl;

	}
	// If we have a name/value pair, then we assume we're good. At least one.
	return !namevalues.empty();
}

bool Shell::requiredAttributesPresent(string& subcmd, const char* attr1, const char* attr2, const char* attr3, const char* attr4)
{
	// extract the destination server
	if (namevalues.empty())
		return false;

	if (namevalues[attr1]=="")
	{
		cout << subcmd << " Error. Requires attribute '" << attr1 << "'" << endl;
		return false;
	}

	if (attr2!="" && namevalues[attr2]=="")
	{
		cout << subcmd << " Error. Requires attribute '" << attr2 << "'" << endl;
		return false;
	}

	if (attr3!="" && namevalues[attr3]=="")
	{
		cout << subcmd << " Error. Requires attribute '" << attr3 << "'" << endl;
		return false;
	}

	if (attr4!="" && namevalues[attr4]=="")
	{
		cout << subcmd << " Error. Requires attribute '" << attr4 << "'" << endl;
		return false;
	}

	return true;
}

bool Shell::processLocal(string& cmd, string& subcmd)
{
	assert(cmd=="LOCAL");

	if (subcmd=="GENERATEKEYPAIR")
	{
		if (ssh.hasLocalKeypair())
		{
			cout << "We have a keypair. No need to generate." << endl;
			return false;
		}
		else
		{
			ssh.generateLocalKeypair();

			if (!ssh.hasLocalKeypair())
			{
				cout << "ERROR: Keypair generation FAILED. Exiting." << endl;
				return false;
			}
		}
	}
	else if (subcmd=="SENDPUBLICKEY" || subcmd=="TESTAUTHENTICATION")
	{
		if (!requiredAttributesPresent(subcmd, "sfipaddress"))
		{
			cout << "ERROR: Required attribute 'sfipaddress' not found." << endl;
			return false;
		}

		string ipaddr;

		ipaddr = namevalues["sfipaddress"];

		cout << "IP address = '" << ipaddr << "'" << endl;;

		if (subcmd=="SENDPUBLICKEY")
		{
			cout << " Pushing public key to remote server..." << endl;
			return ssh.pushLocalPublicKey(ipaddr);
		}
		else
		{
			cout << " Testing secure connection and authentication with remote server..." << endl;
			return ssh.testAuthentication(ipaddr);
		}

	}
	else if (subcmd=="CREATESESSION" || subcmd=="REMOVESESSION")
	{
		if (!requiredAttributesPresent(subcmd, "sessid"))
		{
			cout << "ERROR: Required attribute 'sessid' not found." << endl;
			return false;
		}

		stringstream sess_str;
		sess_str.str(namevalues["sessid"]);
		int sess_id;

		sess_str >> sess_id;
		ios::iostate st;

		st = sess_str.rdstate();

		if (st & (ifstream::badbit | ifstream::failbit))		// An error of some sort occured. Likely sess_id has no valid id due to contents.
		{
			cout << "In " << subcmd << " sessid='" << sess_str.str() << "' is not a number. Error." << endl;
			return false;
		}

		if (subcmd=="CREATESESSION")
		{
			int ret;

			cout << "Creating session ID=" << sess_id << endl;
			ret = local.AddSession(sess_id);
			if (ret)
			{
				switch(ret)
				{
				case ID_IN_USE:
					cout << "ERROR: Session create of id=" << sess_id << " failed. ID IN USE." << endl;
					break;
				default:
					cout << "ERROR: Session create of id=" << sess_id << " failed with errno=" << ret << endl;
					break;
				}
			}
			else
				cout << "Session create successful." << endl;
		}
		else
		{
			int ret;

			cout << "Deleting session ID=" << sess_id << endl;
			ret = local.RemoveSession(sess_id);
			if (ret)
				cout << "ERROR: Removal of session " << sess_id << " failed with errno=" << ret << endl;
			else
				cout << "Session removal successful." << endl;
		}
	}
	else
	{
		cout << "ERROR: LOCAL sub-command '" << subcmd << "' is unknown." << endl;
		return false;
	}

	// We're good if we just 'drop' to here.
	return true;
}

bool Shell::processCommand(string& cmd, string& subcmd)
{

	// Convert cmd and subcmd to upper case for further processing as XML elements.
	strtoupper(cmd);
	strtoupper(subcmd);

//	switch(cmd.c_str())
	{
		if (cmd=="HELP" || cmd=="?")
		{
		cout << endl << "uBrain -- Valid Commands are:" << endl << endl
			<< "QUIT|EXIT" << endl
			<< endl
			<< "STATUS FACTORIES|ENTITIES|ENDPOINTS" << endl
			<< endl
			<< "CONNECT srcname=<name> srcip=<ipaddress> destip=<ipaddress>" << endl
			<< endl
			<< "SF ADDSESSION" << endl
			<< "SF ADDENTITY" << endl
			<< "SF DELETEENTITY" << endl
			<< "SF DELETESESSION" << endl
			<< "SF DELETEFACTORY" << endl
			<< endl
			<< "LOCAL CREATESF sfipaddress=<ipaddress> sfid=<sf-id>" << endl << "               [sfname=<human-readable-referencename>]" << endl
			<< "LOCAL CREATESESSION sessid=<sessionid>" << endl
			<< "LOCAL REMOVESESSION sessid=<sessionid>" << endl
			<< "LOCAL REMOVESF sfid=<sf-id>" << endl
			<< "LOCAL KILLSF sfid=<sf-id>" << endl
			<< "LOCAL CREATEKEYPAIR" << endl
			<< "LOCAL SENDPUBLICKEY sfipaddress=<ipaddress>" << endl
			<< "LOCAL TESTAUTHENTICATION sfipaddress=<ipaddress>" << endl
			<< endl
			<< "SETUP VERBOSITY level=<0-10>"
			<< endl << endl;
		return true;
		}

		if (cmd=="EXIT" || cmd=="QUIT")
			exit(0);

		if (cmd=="LOCAL")
		{
			if (!processLocal(cmd, subcmd))
			{
				cout << "Error executing: LOCAL " << subcmd << endl;
				return false;
			}
			else
				return true;
		}

//	case "STATUS": case "CONNECT": case "SF": case "SETUP":
		if (cmd=="STATUS" || cmd=="CONNECT" || cmd=="SF" || cmd=="SETUP")
		{
		cout << "Temporarily not executing Command: " << cmd << " <no implementation>." << endl;

		addToXML(cmd, subcmd);

		return true;
		}

//	default:
		cout << endl << "ERROR: Command: " << cmd << " is unknown to the parser." << endl;
		return false;
	}

	// Should never reach here.
	assert(false);
	return true;
}

bool Shell::addToXML(string& cmd, string& subcmd)
{
    int rc;
    xmlTextWriterPtr writer;
    xmlBufferPtr buf;
    xmlChar *tmp;

    /* Create a new XML buffer, to which the XML document will be
     * written */
    buf = xmlBufferCreate();
    if (buf == NULL) {
        printf("testXmlwriterMemory: Error creating the xml buffer\n");
        return false;
    }

    /* Create a new XmlWriter for memory, with no compression.
     * Remark: there is no compression for this kind of xmlTextWriter */
    writer = xmlNewTextWriterMemory(buf, 0);
    if (writer == NULL) {
        printf("testXmlwriterMemory: Error creating the xml writer\n");
        return false;
    }

    /* Start the document with the xml default for the version,
     * encoding ISO 8859-1 and the default for the standalone
     * declaration. */
    rc = xmlTextWriterStartDocument(writer, NULL, "ISO-8859-1", NULL);
    if (rc < 0) {
        printf
            ("testXmlwriterMemory: Error at xmlTextWriterStartDocument\n");
        return false;
    }

    /* Start an element named "EXAMPLE". Since thist is the first
     * element, this will be the root element of the document. */
    rc = xmlTextWriterStartElement(writer, BAD_CAST cmd.c_str());
    if (rc < 0) {
        printf
            ("testXmlwriterMemory: Error at start of CMD element\n");
        return false;
    }

    rc = xmlTextWriterStartElement(writer, BAD_CAST subcmd.c_str());
    if (rc < 0) {
        printf
            ("testXmlwriterMemory: Error at start of SUBCMD element\n");
        return false;
    }

    //
    // TODO BOGUS attribute being written. Must parse entire 'attributes' string for name/value with '"' marks as well.
    //
    /* Add an attribute with name "xml:lang" and value "de" to ORDER. */
    if (!namevalues.empty())
    {
		  attrNameValuePairs::iterator it;

		  for ( it=namevalues.begin() ; it != namevalues.end(); it++ )
		  {
		    cout << (*it).first << " => " << (*it).second << endl;

		    rc = xmlTextWriterWriteAttribute(writer, BAD_CAST (*it).first.c_str(),
		                                     BAD_CAST (*it).second.c_str());
		    if (rc < 0) {
		        printf
		            ("testXmlwriterMemory: Error at xmlTextWriterWriteAttribute\n");
		        return false;
		    }
		  }
    }

    //
    // Wrap it up...
    //

    /* Here we could close the elements ORDER and EXAMPLE using the
     * function xmlTextWriterEndElement, but since we do not want to
     * write any other elements, we simply call xmlTextWriterEndDocument,
     * which will do all the work. */
    rc = xmlTextWriterEndDocument(writer);
    if (rc < 0) {
        printf("testXmlwriterMemory: Error at xmlTextWriterEndDocument\n");
        return false;
    }

    // Now print it or write it...
    printf("XML Command:\n%s\n", (const char *) buf->content);

    xmlFreeTextWriter(writer);
    xmlBufferFree(buf);

	return true;
}
