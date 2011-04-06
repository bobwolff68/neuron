/*
 * registration.cpp
 *
 *  Created on: Nov 12, 2010
 *      Author: rwolff
 */

#include "registration.h"
#include <cstring>	// Necessary for memcpy

// Our static refcount being initialized for CurlGlobal::
int CurlGlobal::refCount = 0;

RegistrationClient::RegistrationClient(const char* pIp_address, int sfid, int portnum, bool bIsEndpoint, const char* friendlyname)
{
	// TODO Auto-generated constructor stub
	stringstream intconv;

	publicPairs.clear();

	ip_address = pIp_address;
	port_to_use = portnum;

	response[0] = 0;
	respLength = 0;

	// Form URL
	url = "http://" + ip_address;
	url += ":";
	intconv << portnum;

	url += intconv.str();

	if (bIsEndpoint)
	{
		assert(friendlyname);
		url += "/neuron-ep?ep_friendly_name=\"";
		url += friendlyname;
		url += "\"";
	}
	else
	{
		url += "/neuron-sf?sfid=";
		url += ToString<int>(sfid);
	}

	cout << "URL being formed is:" << endl << "  '" << url << "'" << endl;

	if (!setupNetwork())
		throw;

}

RegistrationClient::~RegistrationClient()
{
	// TODO Auto-generated destructor stub

	if (hCurl)
		curl_easy_cleanup(hCurl);
}

size_t RegistrationClient::CURLWriteCallback(void *ptr, size_t size, size_t nmemb, void *ourpointer)
{
  size_t realsize = size * nmemb;
  char* pDest;
  RegistrationClient* pParent;

  assert(ourpointer);
  assert(size + realsize < 1000);

  pParent = (RegistrationClient*)ourpointer;

  if (pParent->respLength==0)
	  pDest = pParent->response;
  else
	  pDest = &(pParent->response[pParent->respLength + 1]);

  memcpy(pDest, ptr, realsize);

  pParent->respLength += realsize;

  return realsize;
}

bool RegistrationClient::registerClient(void)
{
	CURLcode retcode;

	response[0] = 0;
	respLength = 0;
	publicPairs.clear();

	retcode = curl_easy_perform( hCurl );

	if (retcode != CURLE_OK)
	{
		cerr << "CURL_ERROR: " << curl_easy_strerror( retcode ) << endl;
		return false;
	}
	else
	{
		stringstream bodystr;
		string name;
		string value;
		char buffer[100];

		// Now the response body is in our buffer 'response' -- parse it for name/value pairs.

		bodystr.write(response, respLength);

		cout << "Init response: '" << bodystr.str() << "'" << endl;

		while (bodystr.good())
		{
			// Get front end of string and eat the '=' sign.
			bodystr.getline(buffer, 99, '=');
			name = buffer;

			// If this happens, we are likely at end of body at this point....
			// We could simply 'break' here, but if there is a blank line for some reason...
			if (name=="")
				continue;

			// Now read the balance of the line -- the value.
			if (bodystr.peek()=='\"')
			{
				bodystr.get();
				bodystr.getline(buffer, 99, '"');
			}
			else
				bodystr.getline(buffer, 99);

			value = buffer;
//			cout << "Response: Parsed: " << name << "=" << value << endl;

			publicPairs[name.c_str()]=value;
		}

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
	// Do not return the headers to me. Only the body.
	curl_easy_setopt( hCurl, CURLOPT_HEADER, 0);
	curl_easy_setopt( hCurl, CURLOPT_FILE, stdout);

	// Now setup the callback function and the userData pointer to 'this'
	curl_easy_setopt( hCurl, CURLOPT_WRITEFUNCTION, CURLWriteCallback);
	curl_easy_setopt( hCurl, CURLOPT_WRITEDATA, (void*)this);

	return true;
}
