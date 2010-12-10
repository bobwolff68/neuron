#include "neuroncommon.h"
#include <iostream>
#include <sstream>

#include "parsecmd.h"
#include "sshmgmt.h"
#include "shell.h"
#include "registration.h"
#include "ubrainmanager.h"
#include "regserver.h"

using namespace std;

int main(int argc, char** argv)
{
	int gstun_id_ubrain;
	Shell shell;
	uBrainManager ubrain(0);
	map<string, string> respvals;

	RegServer regSrv(&ubrain, respvals);
	gstun_id_ubrain = regSrv.getBrainGStun();

	// Static items -- I think they are static?

	// Should come from command line.
	respvals["stun_ip"] = "207.145.121.125";
	respvals["brain_id"] = "wan://1@blah blah";
	respvals["wan_id"] = "wan://1@ubrain-ip-address::blah blah";


#define TRYCLIENT
#ifdef  TRYCLIENT
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
	cout << "Requesting client (EP) registration in 1 second." << endl;
	sleep(1);
	//
	// Endpoint registration 'true' and friendly name passed.
	//
	RegistrationClient client2("192.168.46.78", 8181, true, "BobClient Test Computer");
	client2.registerClient();
	cout << endl << endl << "uBrain:: response of gstun_id==" << client2.publicPairs["gstun_id"] << endl;
#endif

	//
	//
	//
	while (!cin.eof() && shell.parseLine(cin))
		;

	cout << "\nAnd we are done." << endl;
	//
	//
	//	ssh.setDefaultLocation("/home/rwolff/.ssh/id_rsa_test2");


	return 0;
}

