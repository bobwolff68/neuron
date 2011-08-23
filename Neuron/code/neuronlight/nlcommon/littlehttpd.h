/*
 * LittleHttpd.h
 *
 *  Created on: Dec 3, 2010
 *      Author: rwolff
 */

#ifndef LITTLEHTTPD_H_
#define LITTLEHTTPD_H_

#include "ThreadSingle.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <errno.h>

class LittleHttpd : ThreadSingle
{
public:
	LittleHttpd(map<string, string> rvals, int initport=8081);
	virtual ~LittleHttpd();
	int workerBee(void);
private:
//	bool bIsServerUp;
	int serverError;
	int serversock;
	int port;
	map<string, string> respvalues;
	map<string, string> reqParameters;

	void Init();
	void ShutdownServer(void);
	bool HConnection(int csock);
    
	bool ParseRequest(const char* req) = 0;
    bool ExecuteAction(void) = 0;
    
    void SendBadRequestResponse(void);
    void SendOKResponse(const char* pBody);
    
	void AddToStream(stringstream& outstream, const char* respname);
	//  Now we have a sigint handler simply for the blocking accept() to avoid non-blocking I/O for now.
	static void sighandler(int sig)	{ /*printf("   Signal catcher called for signal %d", sig);*/ return; };
	static void SigIgnore(int sig)	{ printf("   IGNORE Signal for signal %d", sig); return; };
};

#endif /* LittleHttpd_H_ */
