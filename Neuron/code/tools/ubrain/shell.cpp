/*
 * shell.cpp
 *
 *  Created on: Nov 9, 2010
 *      Author: rwolff
 */

#include "shell.h"

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
	string cmd, subcmd, attributes;
	bool eol;

	cout << endl << "uBrain >" << flush;

	while (getline(input, line)){
		if (line=="" || line == "\n" || line == "\r")
		{
			cout << "uBrain >" << flush;
			continue;
		}

//		cout << "line: " << line << endl;

		cmd = "";
		subcmd = "";
		attributes = "";

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

			if (attr[0]==' ')
				attributes = &attr[1];  // Skip the leading space
			else
				attributes = attr;  // No space to skip?

			eol=true;
		}

//		cout << "Command: " << cmd << " -- SubCommand: " << subcmd << " -- Attributes: " << attributes << endl;

		// Now process.
		if (cmd!="")
			processCommand(cmd, subcmd, attributes);

		cout << "uBrain >" << flush;
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

bool Shell::processCommand(string& cmd, string& subcmd, string& attributes)
{

	// Convert cmd and subcmd to upper case for further processing as XML elements.
	strtoupper(cmd);
	strtoupper(subcmd);

//	switch(cmd.c_str())
	{
		if (cmd=="HELP")
		{
		cout << endl << "uBrain -- Valid Commands are:" << endl
			<< "HELP -- This message." << endl
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
			<< "LOCAL CREATESF sfipaddress=<ipaddress> sfid=<sf-id> sfname=<human-readable-referencename>"
			<< "LOCAL CREATESESSION"
			<< "LOCAL REMOVESESSION"
			<< "LOCAL REMOVESF"
			<< "LOCAL KILLSF"
			<< endl
			<< "SETUP CREATEKEYPAIR"
			<< "SETUP SENDPUBLICKEY"
			<< "SETUP TESTAUTHENTICATION"
			<< "SETUP VERBOSITY level=<0-10>"
			<< endl << endl;
		return true;
		}

		if (cmd=="EXIT" || cmd=="QUIT")
			exit(0);

//	case "STATUS": case "CONNECT": case "SF": case "LOCAL": case "SETUP":
		if (cmd=="STATUS" || cmd=="CONNECT" || cmd=="SF" || cmd=="LOCAL" || cmd=="SETUP")
		{
		cout << "Temporarily not executing Command: " << cmd << " <no implementation>." << endl;
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
