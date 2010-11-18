/*
 * registration.cpp
 *
 *  Created on: Nov 12, 2010
 *      Author: rwolff
 */

#include "registration.h"

// Our static refcount being initialized for CurlGlobal::
int CurlGlobal::refCount = 0;

RegistrationClient::RegistrationClient(const char* pIp_address, int portnum)
{
	// TODO Auto-generated constructor stub

	ip_address = pIp_address;
	port_to_use = portnum;

	// Form URL
	url = "http://" + ip_address;
	url += "/index.html";

	if (!setupNetwork())
		throw;

}

RegistrationClient::~RegistrationClient()
{
	// TODO Auto-generated destructor stub

	if (hCurl)
		curl_easy_cleanup(hCurl);
}


bool RegistrationClient::registerClient(void)
{
	CURLcode retcode;

	retcode = curl_easy_perform( hCurl );

	if (retcode != CURLE_OK)
	{
		cerr << "CURL_ERROR: " << curl_easy_strerror( retcode ) << endl;
		return false;
	}
	else
	{
		// could get CURLINFO_ ...
		//  HTTP_CODE
		//  CONTENT_TYPE
		//  SIZE_DOWNLOAD
		//  SPEED_DOWNLOAD

		return true;
	}
}

bool RegistrationClient::setupNetwork(void)
{
	hCurl = curl_easy_init();
	if (!hCurl)
	{
		cerr << "Init of CURL library curl_easy_init() failed." << endl;
		return false;
	}

#if 0
//#ifdef DEBUG
	curl_easy_setopt( hCurl, CURLOPT_VERBOSE, true );
#endif

	curl_easy_setopt( hCurl, CURLOPT_URL, url.c_str());
	curl_easy_setopt( hCurl, CURLOPT_NOPROGRESS, true );

//Skip headers altogether.	curl_easy_setopt( hCurl, CURLOPT_WRITEHEADER, stdout);
	curl_easy_setopt( hCurl, CURLOPT_FILE, stdout);

	return true;
}



RegistrationServer::RegistrationServer()
{
	// TODO Auto-generated constructor stub

}

RegistrationServer::~RegistrationServer()
{
	// TODO Auto-generated destructor stub
}




