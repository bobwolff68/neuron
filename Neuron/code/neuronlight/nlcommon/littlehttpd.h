/*
 * LittleHttpd.h
 *
 *  Created on: Dec 3, 2010
 *      Author: rwolff
 */

#ifndef LITTLEHTTPD_H_
#define LITTLEHTTPD_H_

#include "ThreadSingle.h"

#include <map>
#include <iostream>
#include <sstream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <errno.h>

class LittleHttpd : public ThreadSingle
{
public:
	LittleHttpd(map<string, string> rvals, int initport=8081);
	virtual ~LittleHttpd();
	int workerBee(void);
    bool isServerRunning(void) { return bIsServerUp; };

protected:
    bool RequiredArgPresent(const char* reqarg);
    bool getRequiredArgAsInt(const char* reqarg, int low, int high, int& outint);
    int StrToInt(string& str)
    {
        std::stringstream strstr(str);
        int i;
        
        strstr >> i;
        
        return i;
    }

    string bodyToReturn;
    string fullInboundURL;
    string inboundBaseURL;
    map<string,string> inboundPairs;
    
private:
	bool bIsServerUp;
    bool bInitComplete;
	int serverError;
	int serversock;
	int port;

    char clientIpAddress[INET6_ADDRSTRLEN];
	map<string, string> respvalues;
	map<string, string> reqParameters;

	void Init();
	void ShutdownServer(void);
	bool HConnection(int csock);
    void AutoParse(void);
    
	virtual bool ParseRequest(void) = 0;
    virtual bool ExecuteAction(void) = 0;
    
    void SendBadRequestResponse(int csock, string &body);
    void SendOKResponse(int csock, string &body);

	//  Now we have a sigint handler simply for the blocking accept() to avoid non-blocking I/O for now.
	static void sighandler(int sig)	{ /*printf("   Signal catcher called for signal %d", sig);*/ return; };
	static void SigIgnore(int sig)	{ printf("   IGNORE Signal for signal %d", sig); return; };
};

#endif /* LittleHttpd_H_ */
