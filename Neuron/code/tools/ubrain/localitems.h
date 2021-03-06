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

class SFInfo
{
public:
	SFInfo() { num_sessions=0; curSFState=com::xvd::neuron::OBJECT_STATE_STANDBY; };
	virtual ~SFInfo() { };
	int sf_id;
    int acp_slave_wan_id;       // Global ID for each SF for the STUN connection
    int scp_slave_wan_id;       // Global ID for each SF for the STUN connection
	string sf_ipaddress;
	string sf_name;
	bool is_endpoint;
	int num_sessions;
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

class SessOnSFInfo
{
public:
    SessOnSFInfo(int sid, pSFInfo initpSF, int wID);
    virtual ~SessOnSFInfo() { };
    pSFInfo pSF;
    int sessid;
    com::xvd::neuron::ObjectState state;
    int media_wan_id;           // Each session on a particular SF will have it's own WAN id. Each of the sessions-on-each-sf will need peers added for these.
};

typedef SessOnSFInfo* pSessOnSFInfo;
typedef map<int,pSessOnSFInfo> SessOnSFInfoList;

class SessInfo
{
public:
	SessInfo();
	virtual ~SessInfo();
	int sess_id;
	SourceInfoList sourceList;
	string sessName;
	SessOnSFInfoList SessionInfoOnSF;
    bool SessOnSFListFind(int sfID) { return (SessionInfoOnSF.find(sfID) != SessionInfoOnSF.end()); };
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
	LocalItems(const char* regsrvip=NULL);
	virtual ~LocalItems();
	void ClearAll(void) { sfList.clear(); sessList.clear(); entList.clear(); };

	int AddSession(int sessID, const char* sessname);
	int AddSFToSession(int sfID, int sessID, int wanID, const char* sessname);
	int GetNumSFsInSession(int sessID) { return SessListFind(sessID) ? sessList[sessID]->SessionInfoOnSF.size() : -1; };
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
	int AddSFLaunch(int sfID, const char* ip, const char* name, const char* usernameAt);
	int RemoveSF(int sfID);
	int RemoveSFFromAllSessions(int sfID);
	int RemoveSFFromSession(int sfID, int sessID);
	void ListSFs(void);
    SFInfo* GetSFInfo(int sfid) { if (SFListFind(sfid)) return sfList[sfid]; };
	int GetNumSFs(void) { return sfList.size(); };
	com::xvd::neuron::ObjectState GetCurSFState(int sfid) { if (SFListFind(sfid)) return sfList[sfid]->curSFState; else return (com::xvd::neuron::ObjectState)ID_NOT_FOUND; };
	int UpdateCurSFState(int sfid, com::xvd::neuron::ObjectState state) { if (SFListFind(sfid)) { sfList[sfid]->curSFState = state; return 0; } else return ID_NOT_FOUND;};

	int AddEntity(int entID, int sfID, int sessID, EntInfo::EntType type, const char* entname, int resx, int resy, int src_ent_id=-1);
	int RemoveEntity(int entID);
	void ListEntities(void);
	EntInfo* GetEntInfo(int entID) { if (EntListFind(entID)) return entList[entID]; };
	EntInfo* GetEntInfo(const char* entname) { string en=entname; if (EntListFind(en.c_str())) return entNameList[en]; };

	map<string,EntInfo::EntType> entTypeMap;

	void setRegServerPublicIP(const char* instr) { regServerPublic = instr; };
protected:
    bool SFListFind(int sfid) { return (sfList.find(sfid) != sfList.end()); };
	bool SessListFind(int sessid) { return (sessList.find(sessid) != sessList.end()); };
    bool EntListFind(int entID) { return (entList.find(entID) != entList.end()); };
    bool EntListFind(const char * entname) { string en=entname; return (entNameList.find(en) != entNameList.end()); };
	SFList sfList;
	SessList sessList;
	EntList entList;
	EntNameList entNameList;
	string inoutnull;
	string innull;
	string regServerPublic;
};

#endif /* LOCALITEMS_H_ */
