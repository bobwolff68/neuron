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
	Shell shell;
	uBrainManager ubrain;
	map<string, string> respvals;

	// Static items -- I think they are static?
	respvals["stun_ip"] = "207.145.121.125";
	respvals["brain_id"] = "wan://1@blah blah";
	respvals["wan_id"] = "wan://1@ubrain-ip-address::blah blah";

	RegServer regSrv(&ubrain, respvals);

#define TRYCLIENT
#ifdef  TRYCLIENT
#if 0
	cout << "Requesting client (SF) registration in 2 seconds..." << endl;
	sleep(4);
	RegistrationClient client("192.168.46.78",8181);
	client.registerClient();
#endif

	cout << "Requesting client (EP) registration in 1 second." << endl;
	sleep(1);
	RegistrationClient client2("192.168.46.78", 8181, true, "BobClient Test Computer");
	client2.registerClient();

	cout << "Exiting application in 4 seconds..." << endl;
	sleep(4);
	exit(2);
#endif

	//
	//
	//
	shell.parse(cin);

	cout << "\nAnd we are done." << endl;
	exit(1);

	//
	//
	//	ssh.setDefaultLocation("/home/rwolff/.ssh/id_rsa_test2");


	return 0;
}

