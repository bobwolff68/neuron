#include "neuroncommon.h"
#include <iostream>
#include <fstream>

#include "parsecmd.h"
#include "sshmgmt.h"
#include "shell.h"
#include "registration.h"
#include "ubrainmanager.h"
#include "regserver.h"

extern bool parsecmd(int argc, char**argv);
extern string stunserver;
extern string startupscript;
extern string logoutfile;

int main(int argc, char** argv)
{
	int gstun_id_ubrain;
	Shell* pShell;
	map<string, string> respvals;

	// Static items -- I think they are static?

	// Should come from command line.
	respvals["stun_ip"] = "207.145.121.125";
	respvals["brain_id"] = "wan://1@blah blah";
	respvals["wan_id"] = "wan://1@ubrain-ip-address::blah blah";

	///
	/// Parse the command line and setup variables for options. Then start rolling for real.
	///
	parsecmd(argc, argv);

	if (stunserver != "")
	{
		cout << "Got new stun: " << stunserver << endl;
		respvals["stun_ip"] = stunserver;
	}

	///
	/// Now get rolling
	///
	uBrainManager uBrain(0);

#define TRYCLIENTSERVER
#ifdef  TRYCLIENTSERVER

	RegServer regSrv(&uBrain, respvals, 80);
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

		if (!pShell->parseLine(cin, bIsScript))
			break;
	}

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

