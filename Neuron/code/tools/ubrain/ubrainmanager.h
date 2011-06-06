/*
 * ubrainmanager.h
 *
 *  Created on: Dec 3, 2010
 *      Author: rwolff
 */

#ifndef UBRAINMANAGER_H_
#define UBRAINMANAGER_H_

class uBrainManager;
class MinibrainTestInstance;

#include "neuroncommon.h"
#include "localitems.h"
#include "sshmgmt.h"

#include "controller.h"

class uBrainManager : ThreadSingle, CallbackBase
{
public:
	uBrainManager(int brainId, map<string,string> nvPairs, int domainId=67);
	virtual ~uBrainManager();
	bool GetUniqueSFidAndMarkSFAsCreated(int& sfIdOut);
	bool GetUniqueEntityID(int& entIdOut);
	bool RegistrationComplete(map<string,string> pairs, bool isEP);
	bool processLocal(string& cmd, string& subcmd, map<string, string> & nvPairs);
	bool processDDSOriented(string& cmd, string& subcmd, map<string, string> & nvPairs);
	bool processDDS_SF(string& cmd, string& subcmd, map<string, string> & nvPairs);
	bool ProcessDDS_SF_AddSession(string& cmd, string& subcmd, map<string, string> & nvPairs);
	bool ProcessDDS_SF_AddEntity(string& cmd, string& subcmd, map<string, string> & nvPairs);
	bool ProcessDDS_SF_ChangeConnection(string& cmd, string& subcmd, map<string, string> & nvPairs);
	bool ProcessDDS_SF_DeleteEntity(string& cmd, string& subcmd, map<string, string> & nvPairs);
	bool RefreshSourcesOnSession(int sessID);
	int AddSFInternally(int sfID, const char* ip, int acpID, int scpID, const char* name, bool isEP);

	// Callback items
	void NewSFDetected(int id);
	void NewSFState(com::xvd::neuron::acp::State* state);
	void NewSessionState(com::xvd::neuron::scp::State* state);
	bool MakeLuaCallback(const char* fn, int arg, const char* txt);
	// Associated with Callback
	void ReceiveOfferSource(com::xvd::neuron::scp::State* state);
	void ReceiveSelectSource(com::xvd::neuron::scp::State* state);

	int GetNewGlobalWANID(void) { return globalWANIDMax++; };

	Controller* getController(void) { return pCtrl; };

private:
	int workerBee(void);
	bool requiredAttributesPresent(string& subcmd, map<string,string>& nvPairs, const char* attr1, const char* attr2="", const char* attr3="", const char* attr4="", const char* attr5="");
	string FormulateScript(const char* incmd, string& enttype, int entid, int ent_src, const char* optName=NULL);
	int WaitForSFReady(int sfid, int timeInms);
	int WaitForSessionReadyOnSF(int sessid, int sfid, int timeInms);

	bool bUseLANOnly;
	Controller* pCtrl;
	LocalItems local;       // Will set the registration server next...
	SSHManagement ssh;
	int globalWANIDMax;
};

#endif /* UBRAINMANAGER_H_ */
