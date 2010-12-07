/*
 * ubrainmanager.cpp
 *
 *  Created on: Dec 3, 2010
 *      Author: rwolff
 */

#include "ubrainmanager.h"

uBrainManager::uBrainManager()
{
	// TODO Auto-generated constructor stub

}

uBrainManager::~uBrainManager()
{
	// TODO Auto-generated destructor stub
}

int uBrainManager::workerBee(void)
{
	return 0;
}

bool uBrainManager::GetUniqueSFidAndMarkSFAsCreated(int& sfIdOut)
{
	static int increasingSFID=11555;

	// Here we only dole out a unique id number. We don't do anything with it until RegistrationComplete() is called.
	sfIdOut = increasingSFID++;

	cout << endl << "uBrainManager: sfid=" << sfIdOut << endl;

	return true;
}

///
/// \brief When registration has been completed by RegServer::, it should tell the uBrainManager this fact.
///        The data presented allows the uBrainManager to finalize creation of the SF internally, matching
///        friendly name with sfid/ip address etc. The other crucial piece is the global ID for STUN.
///
bool uBrainManager::RegistrationComplete(int sfid, const char* clientIPAddress, const char* friendlyName, int globalID)
{

	// TODO Here is where we will generate a unique sfid and add the SF to the 'lists' without launching it.

	// TODO - Fill in and correlate all registry data -- and create internal SF for endpoints.

	return true;
}
