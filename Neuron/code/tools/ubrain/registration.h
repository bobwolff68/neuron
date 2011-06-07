
/*
 * registration.h
 *
 *  Created on: Nov 12, 2010
 *      Author: rwolff
 */

#ifndef REGISTRATION_H_
#define REGISTRATION_H_

#include "neuroncommon.h"

#include <iostream>

extern "C" {
	#include "netlib/curl/include/curl/curl.h"
}

//
// Global refcounter class for CURL. All users of CURL must inherit this object.
//   Nothing to be done with it. But it keeps track for calling the curl_global_* items.
//
class CurlGlobal
{
public:
        CurlGlobal() { if (!refCount++) init(); };
        virtual ~CurlGlobal() { if (!--refCount) cleanup(); };
private:
        static int refCount;
//        static void init() { curl_global_cleanup(); };
//        static void cleanup() { curl_global_init(CURL_GLOBAL_ALL); };
        static void cleanup() { curl_global_cleanup(); };
        static void init() { curl_global_init(CURL_GLOBAL_ALL); };
};

//!
//! \class RegistrationClient
//! 
//! \brief 
//! 
//! \todo 
//! 

class RegistrationClient : private CurlGlobal
{
public:
	RegistrationClient(const char* pIp_address, int sfid, int portnum=8181, bool bIsEndpoint=false, const char* friendlyname=NULL);
	virtual ~RegistrationClient();
	bool setupNetwork(void);
	virtual bool registerClient(void);
	bool abort(bool blockingWait);
	// Accessed from 'outside' via the write callback
	char response[1000];
	int respLength;
	map<string, string> publicPairs;
	
	virtual void AbortCallback(void) { cerr << "INFO: Abort completed. Override for custom handler." << endl; };

protected:
    static int CURLProgressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);
    static size_t CURLWriteCallback(void *ptr, size_t size, size_t nmemb, void *ourpointer);
    CURL* hCurl;
	string ip_address;
	int port_to_use;
	string url;
	bool bAbortRequested;
	bool bIsCompleted;
};

//bool RegistrationClient::bCurlReady=false;

class RegistrationClientAsync : public RegistrationClient, public ThreadSingle
{
public:
	RegistrationClientAsync(const char* pIp_address, int sfid, int portnum=8181, bool bIsEndpoint=false, const char* friendlyname=NULL);
	virtual ~RegistrationClientAsync();
	
	bool registerClient(void);
private:	
	int  workerBee(void);
public:	
	// Called with (true) when a real response was received. 
	// If aborted or other curl_error happens, it will be called with (false)
	virtual void ResponseReceived(bool bSuccessful) = 0;
};


#endif /* REGISTRATION_H_ */
