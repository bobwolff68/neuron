/*
 * localitems.h
 *
 *  Created on: Nov 29, 2010
 *      Author: rwolff
 */

#ifndef LOCALITEMS_H_
#define LOCALITEMS_H_

#include "neuroncommon.h"

#define ID_IN_USE			-1
#define ID_NOT_FOUND		-2
#define DEST_SF_NOT_FOUND	-3
#define DEST_SESS_NOT_FOUND	-4
#define DEST_SF_IN_USE		-5
#define TIMEOUT				-6

#define GENERIC_ERROR	-99

//typedef struct SF_s {
//} SFInfo, *pSFInfo;
class SFInfo
{
public:
	SFInfo() { };
	virtual ~SFInfo() { };
	int sf_id;
    int acp_slave_wan_id;       // Global ID for each SF for the STUN connection
    int scp_slave_wan_id;       // Global ID for each SF for the STUN connection
	string sf_ipaddress;
	string sf_name;
	bool is_endpoint;
	com::xvd::neuron::ObjectState curSFState;
};

typedef SFInfo* pSFInfo;
typedef map<int, pSFInfo> SFList;

class SourceInfo
{
public:
	SourceInfo() { };
	virtual ~SourceInfo() { };
	int sfid;
	int sessid;
	string epName;
	string sourceName;	// Source name given at offer-time.
	int entid;		// The actual entity id offered
};

typedef SourceInfo* pSourceInfo;
typedef map<int,pSourceInfo> SourceInfoList;

class SessInfo
{
public:
	SessInfo();
	virtual ~SessInfo();
	int sess_id;
	SFList session_in_these_sfs;
	SourceInfoList sourceList;
	string sessName;
	map<int,com::xvd::neuron::ObjectState> curSessionStateOnSF;
};

typedef SessInfo* pSessInfo;
typedef map<int, pSessInfo> SessList;

class EntInfo
{
public:
	typedef enum {
		Ent_NumSource = 20000,		// Starting out a way to make these stick out.
		Ent_NumSink,
		Ent_RP,
		Ent_FileSource,
		Ent_DecodeSink,
		Ent_SQSink
	} EntType;
	EntInfo() { src_ent_id = -1; };
	virtual ~EntInfo() { };
	int ent_id;
	int src_ent_id;
	int sf_id;
	int sess_id;
	string entname;
	EntInfo::EntType type;		// To be re-defined as enum?

	// Other data needed for an entity?
	int resx, resy;
};

typedef EntInfo* pEntInfo;
typedef map<int, pEntInfo> EntList;
typedef map<string, pEntInfo> EntNameList;

class LocalItems
{
public:
	LocalItems();
	virtual ~LocalItems();
	void ClearAll(void) { sfList.clear(); sessList.clear(); entList.clear(); };

	int AddSession(int sessID, const char* sessname);
	int AddSFToSession(int sfID, int sessID, const char* sessname);
	int GetNumSFsInSession(int sessID) { return sessList[sessID] ? sessList[sessID]->session_in_these_sfs.size() : -1; };
	int RemoveSession(int sessID);
	int AddSourceToSession(int sessID, int sfID, int entID, const char* sourceName);
	int RemoveAllSourcesFromSession(int sessID);
	int RemoveSessionFromSF(int sessID, int sfID);
	void ListSessions(void);
	int ListSourcesInSession(int sessID);
	SessInfo* GetSessionInfo(int sessID);
	bool GetSFsForSession(int sessID, SFList& sfs, bool bEPOnly);
	com::xvd::neuron::ObjectState GetCurStateInSFForSession(int sfid, int sessid);
    int UpdateCurStateInSFForSession(int sfid, int sessid, com::xvd::neuron::ObjectState state);

	int AddSFInternally(int sfID, const char* ip, int acpID, int scpID, const char* name, bool isEP);
	int AddSFLaunch(int sfID, const char* ip, const char* name);
	int RemoveSF(int sfID);
	void ListSFs(void);
    SFInfo* GetSFInfo(int sfid) { return sfList[sfid]; };
	int GetNumSFs(void) { return sfList.size(); };
	com::xvd::neuron::ObjectState GetCurSFState(int sfid) { if (sfList[sfid]) return sfList[sfid]->curSFState; else return (com::xvd::neuron::ObjectState)ID_NOT_FOUND; };
	int UpdateCurSFState(int sfid, com::xvd::neuron::ObjectState state) { if (sfList[sfid]) { sfList[sfid]->curSFState = state; return 0; } else return ID_NOT_FOUND;};

	int AddEntity(int entID, int sfID, int sessID, EntInfo::EntType type, const char* entname, int resx, int resy, int src_ent_id=-1);
	int RemoveEntity(int entID);
	void ListEntities(void);
	EntInfo* GetEntInfo(int entID) { return entList[entID]; };
	EntInfo* GetEntInfo(const char* entname) { string en=entname; return entNameList[en]; };

	map<string,EntInfo::EntType> entTypeMap;

protected:
	SFList sfList;
	SessList sessList;
	EntList entList;
	EntNameList entNameList;
	string inoutnull;
	string innull;
};

#endif /* LOCALITEMS_H_ */
