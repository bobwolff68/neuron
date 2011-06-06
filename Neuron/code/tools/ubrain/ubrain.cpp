#include "neuroncommon.h"
#include <iostream>
#include <fstream>

#include "parsecmd.h"
#include "sshmgmt.h"
#include "shell.h"
#include "registration.h"

#include "ubrainmanager.h"
#include "regserver.h"

#include "luatester.h"

#include <cstring>

#include <readline/readline.h>
#include <readline/history.h>

extern bool parsecmd(int argc, char**argv);
extern string ubrain_ip;
extern string stunserver;
extern string startupscript;
extern string logoutfile;
extern bool bUseLANOnly;
extern string luascripts[16];
extern int luascriptsInUse;

//
// Note that these IDs and strings are in **HEX** but *MUST NOT* have leading "0x" in the string.
//
#define UBRAIN_WAN_ACPID 2
#define UBRAIN_WAN_ACPID_STR "2"
#define UBRAIN_WAN_SCPID 1
#define UBRAIN_WAN_SCPID_STR "1"

extern lua_State* SetupLua(void);

int main(int argc, char** argv)
{
	Shell* pShell;
	map<string, string> respvals;
	lua_State* ubrainLua;

	// Static items -- I think they are static?
#if 0
    NDDSConfigLogger::get_instance()->
        set_verbosity_by_category(NDDS_CONFIG_LOG_CATEGORY_API,
                                  NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
#endif

    ///
    /// Parse the command line and setup variables for options. Then start rolling for real.
    ///
    parsecmd(argc, argv);
   
    // Should come from command line.
    respvals["ubrain_ip"] = ubrain_ip;

    if (bUseLANOnly)
    {
        respvals["use_lan_only"] = "true";
        stunserver = "";
    }
    else
    {
        respvals["stun_ip"] = stunserver;

        respvals["ubrain_acp_desc"] = "[0]@wan://::" UBRAIN_WAN_ACPID_STR ":1.1.1.1";
        respvals["ubrain_scp_desc"] = "[1]@wan://::" UBRAIN_WAN_SCPID_STR ":1.1.1.1";
        respvals["ubrain_acp_id"] = ToString<int>(UBRAIN_WAN_ACPID);
        respvals["ubrain_scp_id"] = ToString<int>(UBRAIN_WAN_SCPID);
    }
	///
	/// Now get rolling
	///
	uBrainManager uBrain(0, respvals);

#if 0
    cout << endl << "Summary of all lua scripts requested on command line:" << endl;
    for (int iScr=0;iScr<luascriptsInUse;iScr++)
    {
	cout << "Running Script["<<iScr<<"] = "<<luascripts[iScr]<<endl;
	MinibrainTestInstance::getInstance()->RunScript(luascripts[iScr].c_str());
    }
#endif

// Make sure to setup the uBrainManager and the Controller with this immediately.
  ubrainLua = MinibrainTestInstance::getInstance()->getLuaState();
  if (!ubrainLua)
    cout << "MinibrainTestInstance::getLuaState() - Setup failed." << endl;

// Finalize setup of the Lua portion.
    MinibrainTestInstance::getInstance()->setMiniBrain(&uBrain);
    MinibrainTestInstance::getInstance()->setController(uBrain.getController());
    
    if (LUACALL(11, "testing 1 2 3"))
	cout << "C++::returned true" << endl;
    else
	cout << "C++::returned false" << endl;
    
    cout << endl << "Next test..." << endl;
    
    if (LUACALL(23, "yes"))
	cout << "C++::returned true" << endl;
    else
	cout << "C++::returned false" << endl;

#define USE_LOCAL_CLIENTSERVER
#ifdef  USE_LOCAL_CLIENTSERVER

	RegServer regSrv(&uBrain, respvals, 8181);

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
			string check;
			line = readline("uBrain > ");
			check = line;
			ToUpper<string>(check);
			
			// Only add non-blank lines to history and non-exiting commands.
			if (strlen(line) && check!="QUIT" && check!="EXIT")
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
