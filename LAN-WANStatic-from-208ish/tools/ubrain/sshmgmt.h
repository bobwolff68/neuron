/*
 * sshmgmt.h
 *
 *  Created on: Nov 10, 2010
 *      Author: rwolff
 */

#ifndef SSHMGMT_H_
#define SSHMGMT_H_

#include "neuroncommon.h"

#include <iostream>
#include <fstream>

#ifdef _WIN32
#  include "winsock.h"
#else
#  include <netdb.h>
#  include <arpa/inet.h>
#endif

// Generate an rsa 2048 keypair with no passphrase for use in ssh authentication
// ssh-keygen -b 2048 -t rsa -q -N "" -f ~/.ssh/id_rsa

// pull local public key and place it remotely in ~/.ssh/authorized_keys
// cat ~/.ssh/id_rsa.pub | ssh <machine> "cat >>~/.ssh/authorized_keys

class SSHManagement
{
public:
	SSHManagement();
	virtual ~SSHManagement();

	bool hasLocalKeypair(string location="");

	bool generateLocalKeypair(string location="");
	bool pushLocalPublicKey(const string ipdest, string location="");
	bool testAuthentication(const string ipdest, string location="");

	bool validateIpAddress(const string ipAddress);
	bool nameToIP(const string& name, long& long_ip_out, string& string_ip_out);
	void setDefaultLocation(string newloc) { deflocation = newloc; bUseIdentityLocation=true; };
private:
	string deflocation;
	bool bUseIdentityLocation;
};

#endif /* SSHMGMT_H_ */
