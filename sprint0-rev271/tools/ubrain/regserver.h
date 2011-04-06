/*
 * regserver.h
 *
 *  Created on: Dec 3, 2010
 *      Author: rwolff
 */

#ifndef REGSERVER_H_
#define REGSERVER_H_

#include "neuroncommon.h"
#include "ubrainmanager.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <errno.h>

class RegServer : ThreadSingle
{
public:
	RegServer(uBrainManager* initBrain, map<string, string> rvals, int initport=8181);
	virtual ~RegServer();
	int workerBee(void);
private:
	bool bIsEndpoint;
	bool bIsServerUp;
	int serverError;
	int serversock;
	int port;
	uBrainManager* theBrain;
	map<string, string> respvalues;
	map<string, string> reqParameters;

	void Init();
	void ShutdownServer(void);
	bool HConnection(int csock);
	bool ParseRequest(const char* req);
	void AddToStream(stringstream& outstream, const char* respname);
	//  Now we have a sigint handler simply for the blocking accept() to avoid non-blocking I/O for now.
	static void sighandler(int sig)	{ /*printf("   Signal catcher called for signal %d", sig);*/ return; };
	static void SigIgnore(int sig)	{ printf("   IGNORE Signal for signal %d", sig); return; };
};

#endif /* REGSERVER_H_ */
