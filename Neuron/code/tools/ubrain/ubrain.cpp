#include "neuroncommon.h"
#include <iostream>
#include <sstream>

#include "parsecmd.h"
#include "sshmgmt.h"
#include "shell.h"
#include "registration.h"

using namespace std;

//
/*

  TODO determine command line options/switches for startup conditioning and possibly pointing to script files(macros)
  
  TODO 

*/
//

int main(int argc, char** argv)
{
	SSHManagement ssh;
	Shell shell;
#ifdef TRYCLIENT
	RegistrationClient client("192.168.46.30",80);


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

	if (ssh.hasLocalKeypair())
	{
		cout << "We have a keypair." << endl;
	}
	else
	{
		ssh.generateLocalKeypair();

		if (!ssh.hasLocalKeypair())
		{
			cout << "ERROR: Keypair generation FAILED. Exiting." << endl;
			return -1;
		}
	}

	string ipaddr;

	cout << "Enter IP address of remote site to receive public key ---> ";
	cin >> ipaddr;

	cout << "IP address = '" << ipaddr << "'. Pushing public key..." << endl;

	ssh.pushLocalPublicKey(ipaddr);

	return 0;
}

