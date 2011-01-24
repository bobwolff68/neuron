/*
 * shell.cpp
 *
 *  Created on: Nov 9, 2010
 *      Author: rwolff
 */

#include "shell.h"

Shell::Shell(uBrainManager* pMgr)
{
	assert(pMgr);

	pBrainManager = pMgr;
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
// LOCAL RUNSCRIPT filename=<script_to_run_via_parser>
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
bool Shell::parseLine(istream& input, bool isScript)
{
	string line;
	stringstream linestream;
	string cmd, subcmd;
	stringstream attrstream;

//		if (!isScript)
//			cout << endl << "uBrain > " << flush;

		// Simply getting to EOF should not be a 'fail' for us. It'll get detected by the upper parser.
		if (!getline(input, line))
			return true;

		namevalues.clear();

		if (line[0]=='#')
			return true;

		while (line=="" || line == "\n" || line == "\r")
		{
			return true;
#if 0
			if (!isScript)
				cout << "uBrain > " << flush;

			// Simply getting to EOF should not be a 'fail' for us. It'll get detected by the upper parser.
			if (!getline(input, line))
				return true;
#endif
		}

		if (isScript)
			cout << endl << "uBrain-Script > " << line << endl;
//		else
//			cout << "uBrain > " << flush;

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
		static char attr[100];

		linestream >> cmd;
		ToUpper<string>(cmd);

		// Command with no subcommand nor attributes will be !.good()
		if (linestream.good())
		{
			// Read the sub command now.

			linestream >> subcmd;
			ToUpper<string>(subcmd);

			// Again - subcommand with no attributes will be !.good() here
			if (linestream.good())
			{
				// read rest of line into attr for further processing.
				linestream.getline(attr, 99);

				int st=0;
				while (attr[st]==' ')
					st++;

				parseAttributes(&attr[st]);
			}
		}

//		cout << "Command: " << cmd << " -- SubCommand: " << subcmd << " -- Attributes: " << attributes << endl;

		// Now process.
		if (cmd=="EXIT" || cmd=="QUIT")
			return false;

		if (cmd!="")
			processCommand(cmd, subcmd);

		return true;
}

bool Shell::ProcessScript(const char* fname)
{
	ifstream fin;

	fin.open(fname);
	if (!fin.is_open())
	{
		cerr << "Error: Could not open script: '" << fname << "'" << endl;
		return false;
	}

	cout << "Executing Script:'" << fname << "' Please wait..." << endl;

	while (!fin.eof())
	{
		if (!parseLine(fin, true))
			break;

		sleep(1);		// Slow down script-running...
	}

	fin.close();
	return true;
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
		bool isSingleQuote=false;
		bool isDoubleQuote=false;
		char outerQuote;
		char innerQuote;

		//
		// We now support attribute values enclosed in double-quotes (") as well as single-quotes (')
		// Additionally, within the quoted string, the opposing quote style can be used as a double-quote
		// Examples:
		//  LOCAL CONTROLLERDIRECT command="scp create 1001 6000 'sessname NameHere'"
		// is equal to...
        //  LOCAL CONTROLLERDIRECT command='scp create 1001 6000 "sessname NameHere"'
		//
		if (input.peek()=='\"')
		{
		    isDoubleQuote = true;
		    outerQuote = '\"';
		    innerQuote = '\'';
		}
		else if (input.peek()=='\'')
        {
		    isSingleQuote = true;
		    outerQuote = '\'';
		    innerQuote = '\"';
        }

		if (isDoubleQuote || isSingleQuote)
		{
			while (input.peek()==outerQuote)
			{
				input.get();
				if (input.eof())
					return false;
			}

			// Now first quote is read...now read till last matching quote is found.
			input.getline(buff, 99, outerQuote);

			// Eat all trailing ' ' spaces.
			while (buff[strlen(buff)-1]==' ')
				buff[strlen(buff)-1]=0;

			value = buff;

			// Now - only in the case of outer double-quotes do a search and replace of single quotes to double quotes.
			// Now look for single quotes and if found, substitute with double-quotes.
			while(isDoubleQuote)
			{
			    size_t index=0;

			    index = value.find("'", index);
			    if (index == string::npos)
			        break;
			    else
			        value[index] = '"';
			}

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

bool Shell::processCommand(string& cmd, string& subcmd)
{
	bool isOK;
	// Convert cmd and subcmd to upper case for further processing as XML elements.
	ToUpper<string>(cmd);
	ToUpper<string>(subcmd);

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
			<< "SF ADDSESSION sessid=<sessid> sfid=<sfid> [sessname=<session_name>]" << endl
			<< "SF ADDENTITY entid=<entid> sfid=<sfid> sessid=<sessid> enttype=<RP|FILESOURCE|DECODESINK|NUMSOURCE|NUMSINK|SQSINK> [If SINK, srcentid=<srcent>] [entname=<entityname>]" << endl
			<< "             (for SQSINK also: [maxqueuelength=<sample_window_length-default=100>]" << endl
			<< "SF CHANGECONNECTION sfid=<sfid> sessid=<session_id> entid=<sink_ent_id> srcentid=<src_ent_id> [sfipaddress=<ip_of_SF>]" << endl
			<< "SF DELETEENTITY entid=<entid>" << endl
			<< "SF DELETESESSION sessid=<sessid>" << endl
			<< "SF DELETEFACTORY sfid=<sfid>" << endl
            << "SF WAITFORSFREADY sfid=<sfid> timeout=<#ms_to_wait>" << endl
            << "SF WAITFORSESSREADY sessid=<sessid> sfid=<sfid> timeout=<#ms_to_wait>" << endl
			<< endl
			<< "LOCAL CREATESF sfipaddress=<ipaddress> sfid=<sf-id>" << endl << "               [sfname=<human-readable-referencename>]" << endl
			<< "LOCAL CREATESOURCE entid=<entid> sfid=<sfid> sessid=<sessid> sourcename=<offered_name>" << endl
//			<< "LOCAL KILLSF sfid=<sf-id>" << endl
			<< "LOCAL CREATEKEYPAIR" << endl
			<< "LOCAL SENDPUBLICKEY sfipaddress=<ipaddress>" << endl
			<< "LOCAL TESTAUTHENTICATION sfipaddress=<ipaddress>" << endl
			<< "LOCAL RUNSCRIPT filename=<filename>" << endl
			<< "LOCAL SLEEP ms=<number_of_milliseconds>" << endl
			<< "LOCAL SHELL command=<command_to_run>" << endl
			<< "LOCAL CONTROLLERDIRECT command=<acp/scp command in controller syntax>" << endl
            << "The following items (indented) are deprecated for direct use." << endl
            << "    LOCAL CREATESESSION sessid=<sessionid> [sessname=<session_name>]" << endl
            << "    LOCAL REMOVESESSION sessid=<sessionid>" << endl
            << "    LOCAL REMOVESF sfid=<sf-id>" << endl
            << "    LOCAL SAVESCRIPT filename=<filename>" << endl
            << "    and Also..." << endl
            << "    SETUP VERBOSITY level=<0-10>" << endl
			<< endl;
		return true;
		}

		if (cmd[0]=='#')
			return true;

		if (cmd=="EXIT" || cmd=="QUIT")
			exit(0);

		if (cmd=="LOCAL")
		{
			//
			// A few items MUST be done here in the parser/shell due to the XML tie-in.
			//
			if (subcmd == "SLEEP")
			{
				int ms;

				if (namevalues["ms"]=="")
				{
					cout << "ERROR: Required attribute 'ms' not found." << endl;
					return false;
				}

				ms = FromString<int>(namevalues["ms"], isOK);
				usleep(1000*ms);

				return true;
			}
            else if (subcmd == "SHELL")
            {
                if (namevalues["command"]=="")
                {
                    cout << "ERROR: Required attribute 'command' not found." << endl;
                    return false;
                }

                int retcode;
                retcode = system(namevalues["command"].c_str());
                retcode = WEXITSTATUS(retcode);

                if (retcode)
                    cout << "FAILURE: LOCAL SHELL failed with exit code: " << retcode << endl;

                return true;
            }
			else if (subcmd == "RUNSCRIPT")
			{
				if (namevalues["filename"]=="")
				{
					cout << "ERROR: Required attribute 'filename' not found." << endl;
					return false;
				}

				if (!ProcessScript(namevalues["filename"].c_str()))
				{
					cout << "ERROR: Running script failed." << endl;
					return false;
				}

				return true;
			}
			else if (subcmd == "SAVESCRIPT")
			{
				if (namevalues["filename"]=="")
				{
					cout << "ERROR: Required attribute 'filename' not found." << endl;
					return false;
				}

				if (!xml.SaveScript(namevalues["filename"].c_str()))
				{
					cout << "ERROR: Saving script failed." << endl;
					return false;
				}

				return true;
			}
			else if (subcmd == "GETGROUP")
			{
				string out;
				bool isOK;
				int groupnum = FromString<int>(namevalues["groupnumber"], isOK);

				if (!isOK)
					groupnum = 0;	// Default.

				if (xml.GetGroup(out, groupnum))
					cout << "Group:" << endl << "'" << out << "'" << endl;
				else
				{
					cout << "ERROR: Could not get last group." << endl;
					return false;
				}

				return true;
			}

			if (!pBrainManager->processLocal(cmd, subcmd, namevalues))
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
			// Turn into XML and write to our in-memory document for saving to a script.
			addToXML(cmd, subcmd);

			if (!pBrainManager->processDDSOriented(cmd, subcmd, namevalues))
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
