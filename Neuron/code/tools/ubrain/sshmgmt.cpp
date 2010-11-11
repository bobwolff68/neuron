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

}

SSHManagement::~SSHManagement()
{
	// TODO Auto-generated destructor stub
}

bool SSHManagement::hasLocalKeypair(string in_name)
{
	ifstream testin;
	string fname;

	if (in_name=="")
		fname = "~/.ssh/id_rsa.pub";
	else
		fname = in_name;

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

bool SSHManagement::generateLocalKeypair(void)
{

}

bool SSHManagement::pushLocalKeypair(const string ipdest)
{

	if (ipdest == "" || !validateIpAddress(ipdest))
	{
		cout << "Error: Invalid or missing IP address for keypair." << endl;
		return false;
	}

	// Now we check for a local keypair...
	if (!hasLocalKeypair())
	{
		cout << "Error: No local keypair exists. Use generateKeypair()." << endl;
		return false;
	}

	// Now we push the information to the new machine to allow for authenticated logins.
	cout << "Beginning connection to " << ipdest << " and a password will be required for this attempt." << endl;

	string sshnow;
	sshnow = "awk '{ print $1 \" \" $2 }' ~/.ssh/id_rsa.pub | ssh ";
	sshnow += ipdest;
	sshnow += " \"cat >>~/.ssh/authorized_keys\"";

	system( sshnow.c_str() );

	cout << "Public key sent to remote server. Testing for ssh with NO PASSWORD." << endl;
	cout << "Should not request a password and simply echo back 'Remote Hello from <hostname-remote>'" << endl;

	sshnow = "ssh ";
	sshnow += ipdest;
	sshnow += " \"echo -n \"Remote Hello from \";cat /etc/hostname";
	system( sshnow.c_str() );

	cout << "Complete. If you were asked for a password on the test-run, then there are problems with automatic authentication." << endl;
}
