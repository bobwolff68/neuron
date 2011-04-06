/*
 * sshmgmt.cpp
 *
 *  Created on: Nov 10, 2010
 *      Author: rwolff
 */

#include "sshmgmt.h"


SSHManagement::SSHManagement()
{
	// TODO Auto-generated constructor stub

	// Setup deflocation.
	deflocation = getenv("HOME");
	deflocation += "/.ssh/id_rsa";

	// If default location is modified, ssh must authenticate using this as well.
	bUseIdentityLocation = false;
}

SSHManagement::~SSHManagement()
{
	// TODO Auto-generated destructor stub
}

bool SSHManagement::hasLocalKeypair(string location)
{
	ifstream testin;
	string fname;

	if (!location.empty())
		fname = location;
	else
		fname = deflocation;

	fname += ".pub";

	testin.open(fname.c_str(), ifstream::in);
	if( testin.is_open() )
	{
		testin.close();
		return true;
	}

	testin.close();
	return false;
}

bool SSHManagement::validateIpAddress(const string ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress.c_str(), &(sa.sin_addr));
    return result != 0;
}

bool SSHManagement::nameToIP(const string& name, long& long_ip_out, string& string_ip_out)
{
    struct hostent *he;

    he = gethostbyname (name.c_str());
    if (he == NULL)
    {
        switch (h_errno)
        {
            case HOST_NOT_FOUND:
                cout << "The host was not found.\n";
                break;
            case NO_ADDRESS:
                cout << "The name is valid but it has no address.\n";
                break;
            case NO_RECOVERY:
                cout << "A non-recoverable name server error occurred.\n";
                break;
            case TRY_AGAIN:
                cout << "The name server is temporarily unavailable.";
                break;
        }

        long_ip_out = 0;
        string_ip_out = "";
        return false;
    }

//TODO    long_ip_out = he->h_addr_list;
    string_ip_out = inet_ntoa (*((struct in_addr *) he->h_addr_list[0]));

    return true;
}

bool SSHManagement::generateLocalKeypair(string location)
{
	string mylocation;

	if (!location.empty())
		mylocation = location;
	else
		mylocation = deflocation;

	// Now we check for a local keypair...
	if (hasLocalKeypair(mylocation))
	{
		cout << "Error: Local keypair already exists. You may now push keypair to remote location." << endl;
		return false;
	}

	cout << "Generating an RSA keypair." << endl;

	//
	// ssh-keygen -b 2048 -t rsa -q -N "" -f ~/.ssh/id_rsa
	//
	string sshnow;

	sshnow = "ssh-keygen -b 2048 -t rsa -q -N \"\" -f ";
	sshnow += mylocation;
	system( sshnow.c_str() );

	// Now we check for a local keypair...
	if (!hasLocalKeypair(mylocation))
	{
		cout << "Error: Keypair does NOT exist. Generating keypair FAILED." << endl
				<< "PLEASE CHECK MANUALLY BEFORE PROCEEDING." << endl;
		return false;
	}

	cout << "Success. Keypair created. You may now push the public key to the remote site(s)." << endl;
	return true;
}

bool SSHManagement::pushLocalPublicKey(const string ipdest, string location)
{
	string mylocation;

	if (!location.empty())
		mylocation = location;
	else
		mylocation = deflocation;

	if (ipdest == "" || !validateIpAddress(ipdest))
	{
		cout << "Error: Invalid or missing IP address for keypair." << endl;
		return false;
	}

	// Now we check for a local keypair...
	if (!hasLocalKeypair(mylocation))
	{
		cout << "Error: No local keypair exists. Use generateKeypair()." << endl;
		return false;
	}

	// Now we push the information to the new machine to allow for authenticated logins.
	cout << "Beginning connection to " << ipdest << "." << endl
			<< " A password will be required if no prior authentication has been setup." << endl;

	string sshnow;
	sshnow = "cat ";
	sshnow += mylocation;
	sshnow += ".pub | ssh ";
	if (bUseIdentityLocation)
	{
		sshnow += " -i ";
		sshnow += mylocation;
		sshnow += " ";
	}
	sshnow += ipdest;
	sshnow += " \"cat >>~/.ssh/authorized_keys\"";

	cout << "*********************" << endl;
	system( sshnow.c_str() );
	cout << "*********************" << endl;

	cout << endl << "Public key sent to remote server." << endl;

	return true;
}

bool SSHManagement::testAuthentication(const string ipdest, string location)
{
	string mylocation;
	string sshnow;

	if (!location.empty())
		mylocation = location;
	else
		mylocation = deflocation;

	if (ipdest == "" || !validateIpAddress(ipdest))
	{
		cout << "Error: Invalid or missing IP address for keypair." << endl;
		return false;
	}

	// Now we check for a local keypair...
	if (!hasLocalKeypair(mylocation))
	{
		cout << "Error: No local keypair exists. Use generateKeypair()." << endl;
		return false;
	}

	cout << "Testing for ssh connection with NO PASSWORD." << endl;
	cout << " Should NOT request a password and simply echo back the following:" << endl
			<< "    'Remote Hello from <hostname-of-remote-server>'" << endl;

	sshnow = "ssh ";
	if (bUseIdentityLocation)
	{
		sshnow += " -i ";
		sshnow += mylocation;
		sshnow += " ";
	}

	sshnow += ipdest;
	sshnow += " \"echo -n 'Remote Hello from ';cat /etc/hostname\"";

	cout << "*********************" << endl;
	system( sshnow.c_str() );
	cout << "*********************" << endl;

	// TODO - Must test that 'sf' exists remotely - execute "sf --version".
	cout << endl << "Final test -- execution of 'sf' remotely." << endl
			<< " Should echo the version of sf on success. Else 'bash: sf: command not found'" << endl;

	sshnow = "ssh ";
	if (bUseIdentityLocation)
	{
		sshnow += " -i ";
		sshnow += mylocation;
		sshnow += " ";
	}

	sshnow += ipdest;
	sshnow += " \"sf --version\"";

	cout << "*********************" << endl;
	system( sshnow.c_str() );
	cout << "*********************" << endl;

	cout << endl << "Complete. If you were asked for a password on the test-run," << endl
			<< " then there are problems with automatic authentication." << endl;

	return true;
}
