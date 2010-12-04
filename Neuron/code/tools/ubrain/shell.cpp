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
			if (linestream.eof())
				break;

			linestream >> subcmd;
			if (linestream.eof())
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
//	cout << "Input has contents:'" << input.str() << "'." << endl;

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

		if (input.eof())
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
				if (input.eof())
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
	} 	while (input.good());


	// Now iterate the list and print them.
#if 0
	if (!namevalues.empty())
	{
		  attrNameValuePairs::iterator it;

		  for ( it=namevalues.begin() ; it != namevalues.end(); it++ )
		    cout << (*it).first << " => " << (*it).second << endl;

	}
#endif

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

bool Shell::processDDSOriented(string& cmd, string& subcmd)
{

	if (cmd=="STATUS")
	{
		if (subcmd=="FACTORIES")
		{

		}
		else if (subcmd=="ENTITIES")
		{

		}
		else if (subcmd=="ENDPOINTS")
		{

		}
		else
		{
			cout << "Unrecognized Command: " << cmd << " " << subcmd << endl;
			return false;
		}
	}
	else if (cmd=="CONNECT")
	{

	}
	else if (cmd=="SF")
	{

	}
	else if (cmd=="SETUP")
	{

	}
	else
	{
		cout << "We should never arrive here - bad command in processDDSOriented()" << endl;
		assert("We should never arrive here - bad command in processDDSOriented()"==NULL);
		return false;
	}

	return true;
}

bool Shell::StreamToInt(stringstream& sstream, int& outInt)
{
	sstream >> outInt;

	if (!sstream)		// An error of some sort occured. Likely sess_id has no valid id due to contents.
	{
		cout << "Syntax Error: '" << sstream.str() << "' is not a number." << endl;
		return false;
	}
	else
		return true;
}

bool Shell::processLocal(string& cmd, string& subcmd)
{
	stringstream sstr;
	int sess_id;
	int sf_id;

	string sfip = namevalues["sfipaddress"];
	string sfname = namevalues["sfname"];

	assert(cmd=="LOCAL");

	sstr.str(namevalues["sessid"]);
	if (!StreamToInt(sstr, sess_id))
		 return false;

	sstr.str(namevalues["sf_id"]);
	if (!StreamToInt(sstr, sf_id))
		 return false;

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

		if (subcmd=="SENDPUBLICKEY")
		{
			cout << " Pushing public key to remote server..." << endl;
			return ssh.pushLocalPublicKey(sfip);
		}
		else
		{
			cout << " Testing secure connection and authentication with remote server..." << endl;
			return ssh.testAuthentication(sfip);
		}

	}
	else if (subcmd=="CREATESESSION" || subcmd=="REMOVESESSION")
	{
		if (!requiredAttributesPresent(subcmd, "sessid"))
		{
			cout << "ERROR: Required attribute 'sessid' not found." << endl;
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

			cout << "Deleting Session ID=" << sess_id << endl;
			ret = local.RemoveSession(sess_id);
			if (ret)
			{
				switch(ret)
				{
				case ID_NOT_FOUND:
					cout << "ERROR: Removal of session id=" << sess_id << " failed. ID NOT FOUND." << endl;
					break;
				default:
					cout << "ERROR: Removal of session id=" << sess_id << " failed with errno=" << ret << endl;
					break;
				}
			}
			else
				cout << "Session removal successful." << endl;
		}
	}
	else if (subcmd=="CREATESF" || subcmd=="REMOVESF")
	{
		if (subcmd=="CREATESF" && !requiredAttributesPresent(subcmd, "sfipaddress", "sfid"))
		{
			cout << "ERROR: Required attributes 'sfipaddress' and/or 'sfid' not found." << endl;
			return false;
		}

		if (subcmd=="REMOVESF" && !requiredAttributesPresent(subcmd, "sfid"))
		{
			cout << "ERROR: Required attribute 'sfid' not found." << endl;
			return false;
		}

		if (subcmd=="CREATESF")
		{
			int ret;

			cout << "Creating Factory ID=" << sf_id << endl;
			ret = local.AddSF(sf_id, sfip.c_str(), sfname.c_str());
			if (ret)
			{
				switch(ret)
				{
				case ID_IN_USE:
					cout << "ERROR: Factory create of id=" << sf_id << " failed. ID IN USE." << endl;
					break;
				case GENERIC_ERROR:
					cout << "ERROR: Factory create of id=" << sf_id << " failed. Likely call to ssh failed." << endl;
					break;
				default:
					cout << "ERROR: Factory create of id=" << sf_id << " failed with errno=" << ret << endl;
					break;
				}
			}
			else
				cout << "Factory create successful. Launch successful." << endl;
		}
		else
		{
			int ret;

			cout << "Deleting Factory ID=" << sf_id << endl;
			ret = local.RemoveSF(sf_id);
			if (ret)
			{
				switch(ret)
				{
				case ID_NOT_FOUND:
					cout << "ERROR: Removal of factory id=" << sf_id << " failed. ID NOT FOUND." << endl;
					break;
				default:
					cout << "ERROR: Removal of factory id=" << sf_id << " failed with errno=" << ret << endl;
					break;
				}
			}
			else
				cout << "Factory removal successful. (Remote launch was not killed via this command.)" << endl;
		}
	}
	else if (subcmd=="KILLSF")
	{
		if (!requiredAttributesPresent(subcmd, "sfid"))
		{
			cout << "ERROR: Required attribute 'sfid' not found." << endl;
			return false;
		}

		// Now kill the remote and remove the factory afterwards.
		// TODO - need to implement an ssh with sed/awk to get the process id or send 'killall' ??
		//        This doesn't seem perfect by any means. Maybe it shouldn't be implemented.

		cout << "****" << endl << "  KILLSF is not implemented yet. Use REMOVESF for local only." << endl << "****" << endl;
		return false;
	}
	else if (subcmd == "SAVESCRIPT")
	{
		if (!requiredAttributesPresent(subcmd, "filename"))
		{
			cout << "ERROR: Required attribute 'filename' not found." << endl;
			return false;
		}

		if (!xml.SaveScript(namevalues["filename"].c_str()))
		{
			cout << "ERROR: Saving script failed." << endl;
			return false;
		}
	}
	else if (subcmd == "GETGROUP")
	{
		string out;
		if (xml.GetGroup(out))
			cout << "Group:" << endl << "'" << out << "'" << endl;
		else
		{
			cout << "ERROR: Could not get last group." << endl;
			return false;
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
			<< "SF ADDSESSION sessid=<sessid> sfid=<sfid>" << endl
			<< "SF ADDENTITY entid=<entid> sfid=<sfid> sessid=<sessid> enttype=<RP|FILESOURCE|DECODESINK> [entname=<entityname>]" << endl
			<< "SF DELETEENTITY entid=<entid>" << endl
			<< "SF DELETESESSION sessid=<sessid>" << endl
			<< "SF DELETEFACTORY sfid=<sfid>" << endl
			<< endl
			<< "LOCAL CREATESF sfipaddress=<ipaddress> sfid=<sf-id>" << endl << "               [sfname=<human-readable-referencename>]" << endl
			<< "LOCAL CREATESESSION sessid=<sessionid>" << endl
			<< "LOCAL REMOVESESSION sessid=<sessionid>" << endl
			<< "LOCAL REMOVESF sfid=<sf-id>" << endl
			<< "LOCAL KILLSF sfid=<sf-id>" << endl
			<< "LOCAL CREATEKEYPAIR" << endl
			<< "LOCAL SENDPUBLICKEY sfipaddress=<ipaddress>" << endl
			<< "LOCAL TESTAUTHENTICATION sfipaddress=<ipaddress>" << endl
			<< "LOCAL SAVESCRIPT filename=<filename>" << endl
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
			cout << "Temporarily NOT executing Command: " << cmd << " <no implementation>." << endl;

			// Turn into XML and write to our in-memory document for saving to a script.
			addToXML(cmd, subcmd);

			if (!processDDSOriented(cmd, subcmd))
			{
				cout << "Error executing DDS commands: " << cmd << " " << subcmd << endl;
				return false;
			}
			else
				return true;

			// TODO We eventually want execution to come from the XML directly and not from my feeble parser I think.
			// TODO Currently we would have to read an xml script, turn it into cmd, subcmd, namevalues[] and processCommand()
/*
			string tags;
			if (!xml.GetGroup(tags))
				return false;

			return ProcessSingleCommand(tags.c_str());
*/
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

    if (!xml.AddCommand(cmd, subcmd, namevalues))
    	return false;

	return true;
}

bool Shell::ProcessSingleCommand(const char* tags)
{
	xmlTextReaderPtr reader;
	int ret;

//#define VALIDATEDTD
#ifdef VALIDATEDTD
    /*
     * Pass some special parsing options to activate DTD attribute defaulting,
     * entities substitution and DTD validation
     */
    reader = xmlReaderForMemory(tags, strlen(tags), NULL, NULL,
                 XML_PARSE_DTDATTR |  /* default DTD attributes */
		 XML_PARSE_NOENT |    /* substitute entities */
		 XML_PARSE_DTDVALID); /* validate with the DTD */
#else
    reader = xmlReaderForMemory(tags, strlen(tags), NULL, NULL, 0);
#endif
    if (reader != NULL) {

        ret = xmlTextReaderRead(reader);
        while (ret == 1) {
            ProcessNode(reader);
            ret = xmlTextReaderRead(reader);
        }

        xmlFreeTextReader(reader);

        // From final ReaderRead() return value...
        if (ret != 0) {
        	cout << "Failed to parse tags buffer" << endl;
        	return false;
        }
    } else {
        cout << "Unable to create reader from tags buffer." << endl;
        return false;
    }
    return true;
}

void Shell::ProcessNode(xmlTextReaderPtr reader)
{

}
