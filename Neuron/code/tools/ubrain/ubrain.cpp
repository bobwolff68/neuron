#include "neuroncommon.h"
#include <iostream>
#include <fstream>

#include "parsecmd.h"
#include "sshmgmt.h"
#include "shell.h"
#include "registration.h"
#include "ubrainmanager.h"
#include "regserver.h"

#include <readline/readline.h>
#include <readline/history.h>

extern bool parsecmd(int argc, char**argv);
extern string stunserver;
extern string startupscript;
extern string logoutfile;

#define UBRAIN_WAN_ACPID 2
#define UBRAIN_WAN_ACPID_STR "2"
#define UBRAIN_WAN_SCPID 1
#define UBRAIN_WAN_SCPID_STR "1"

int main(int argc, char** argv)
{
	int gstun_id_ubrain;
	Shell* pShell;
	map<string, string> respvals;

	// Static items -- I think they are static?
#if 0
    NDDSConfigLogger::get_instance()->
        set_verbosity_by_category(NDDS_CONFIG_LOG_CATEGORY_API,
                                  NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
#endif

	// Should come from command line.
	respvals["stun_ip"] = "207.145.121.125";
    respvals["ubrain_acp_desc"] = "1@wan://::" UBRAIN_WAN_ACPID_STR ":1.1.1.1";
    respvals["ubrain_scp_desc"] = "1@wan://::" UBRAIN_WAN_SCPID_STR ":1.1.1.1";
    respvals["ubrain_acp_id"] = ToString<int>(UBRAIN_WAN_ACPID);
    respvals["ubrain_scp_id"] = ToString<int>(UBRAIN_WAN_SCPID);

	///
	/// Parse the command line and setup variables for options. Then start rolling for real.
	///
	parsecmd(argc, argv);

	if (stunserver != "")
	{
		cout << "STUN Server IP: " << stunserver << endl;
		respvals["stun_ip"] = stunserver;
	}

	///
	/// Now get rolling
	///
	uBrainManager uBrain(0, respvals);

#define TRYCLIENTSERVER
#ifdef  TRYCLIENTSERVER

	RegServer regSrv(&uBrain, respvals, 8181);
	gstun_id_ubrain = regSrv.getBrainGStun();

#if 0
	cout << "Requesting client (SF) registration in 1 second..." << endl;
	sleep(1);
	//
	// NON-endpoint registration. No additional parameters. Even '8181' is not required
	//
	RegistrationClient client("192.168.46.78",8181);
	client.registerClient();
	cout << endl << endl << "uBrain:: response of gstun_id==" << client.publicPairs["gstun_id"] << endl;
#endif

#if 0
	cout << "Requesting client (EP) registration in 1 second." << endl;
	sleep(1);
	//
	// Endpoint registration 'true' and friendly name passed.
	//
	RegistrationClient client2("192.168.46.78", 8181, true, "BobClient Test Computer");
	client2.registerClient();
	cout << endl << endl << "uBrain:: response of gstun_id==" << client2.publicPairs["gstun_id"] << endl;
#endif

#endif

	// Use for re-assigning cin properly at the end.
	std::streambuf * oldBuf = NULL;
	std::filebuf inFileBuf;

	if (startupscript!="")
	{
		// create a new streambuf object that reads from a file
		inFileBuf.open(startupscript.c_str(), std::ios::in);

		if (!inFileBuf.is_open())
		{
			cout << "Error: Cannot open script - '" << startupscript << "'" << endl;
			return 3;
		}
		else
			// set the streambuf object to cout and store the previous streambuf
			oldBuf = std::cin.rdbuf(&inFileBuf);
	}

	pShell = new Shell(&uBrain);
	if (!pShell)
	{
		cout << "FATAL ERROR: uBrainManager() instantiation failed." << endl;
		return 2;
	}

	bool bIsScript = (oldBuf != NULL);
	stringstream readline_cin;
	
	// Setup readline to use history ...
        using_history();
        read_history(NULL); // Load prior history from ~/.history
	
	// Use cin regardless of script or no script. It will all be worked out by the re-assign of rdbuf() above
	while (1)
	{
		// If we are in non-script mode, eof mean's we're done.
		if (cin.eof() && !bIsScript)
			break;

		// In script mode, after script is done...drop back to cin shell.
		if (cin.eof() && bIsScript)
		{
			// Re-assign back to cin and continue parsing.
			bIsScript = false;
			std::cin.rdbuf(oldBuf);
			oldBuf = NULL;
		}

		if (bIsScript)
		{
			if (!pShell->parseLine(cin, bIsScript))
				break;
		}
		else
		{
		// Using readline so we need to use it for getting the line and history. 
		// Then we parse it and move along.
			char *line;
			line = readline("uBrain > ");
			
			if (strlen(line))
			      add_history(line);

			// Now emulate a stream input from cin with the just-received-line
			readline_cin << line << endl;
			
			// Line is malloc()'d by readline.
			free(line);
			
			if (!pShell->parseLine(readline_cin, bIsScript))
				break;
		}
			
	}

    write_history(NULL); // Save history to ~/.history
	// restore original streambuf
	if (oldBuf)
		std::cin.rdbuf(oldBuf);

	cout << "\nAnd we are done." << endl;
	//
	//
	//	ssh.setDefaultLocation("/home/rwolff/.ssh/id_rsa_test2");

	delete pShell;

	return 0;
}

