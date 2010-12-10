/*
 * ubrainmanager.h
 *
 *  Created on: Dec 3, 2010
 *      Author: rwolff
 */

#ifndef UBRAINMANAGER_H_
#define UBRAINMANAGER_H_

#include "neuroncommon.h"

#include "../../control/test/controller.h"

class uBrainManager : ThreadSingle
{
public:
	uBrainManager(int brainId=0, int domainId=67);
	virtual ~uBrainManager();
	bool GetUniqueSFidAndMarkSFAsCreated(int& sfIdOut);
	bool RegistrationComplete(int sfid, const char* clientIPAddress, const char* friendlyName, int globalID);
private:
	int workerBee(void);
	Controller* pCtrl;

};

#endif /* UBRAINMANAGER_H_ */
