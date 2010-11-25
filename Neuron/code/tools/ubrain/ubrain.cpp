#include "neuroncommon.h"
#include <iostream>
#include <sstream>

#include "parsecmd.h"
#include "sshmgmt.h"
#include "shell.h"
#include "registration.h"

using namespace std;

int main(int argc, char** argv)
{
	Shell shell;
//#define TRYCLIENT
#ifdef  TRYCLIENT
	RegistrationClient client("192.168.46.78",8181);


	client.registerClient();
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

