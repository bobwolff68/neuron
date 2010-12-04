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

bool uBrainManager::GetUniqueSFidAndMarkSFAsCreated(int& sfIdOut, const char* clientIPAddress, string& friendlyName, const char* tempID)
{
	static int increasingSFID=11555;

	// TODO Here is where we will generate a unique sfid and add the SF to the 'lists' without launching it.

	sfIdOut = increasingSFID++;

	cout << endl << "uBrainManager: sfid=" << sfIdOut << " and Name=" << friendlyName << endl;

	return true;
}
