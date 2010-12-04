/*
 * registration.h
 *
 *  Created on: Nov 12, 2010
 *      Author: rwolff
 */

#ifndef REGISTRATION_H_
#define REGISTRATION_H_

#include "neuroncommon.h"

#include <string>
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
        static void init() { curl_global_cleanup(); };
        static void cleanup() { curl_global_init(CURL_GLOBAL_ALL); };
};

class RegistrationClient : private CurlGlobal
{
public:
	RegistrationClient(const char* pIp_address, int portnum=8181, bool bIsEndpoint=false, const char* friendlyname=NULL);
	virtual ~RegistrationClient();
	bool setupNetwork(void);
	bool registerClient(void);

protected:
	CURL* hCurl;
	string ip_address;
	int port_to_use;
	string url;
};

//bool RegistrationClient::bCurlReady=false;

#endif /* REGISTRATION_H_ */
