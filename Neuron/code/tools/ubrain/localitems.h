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

#define GENERIC_ERROR	-99

//typedef struct SF_s {
//} SFInfo, *pSFInfo;
class SFInfo
{
public:
	SFInfo() { };
	virtual ~SFInfo() { };
	int sf_id;
	int g_id;		// Global ID for each SF for the STUN connection
	string sf_ipaddress;
	string sf_name;
	bool is_endpoint;
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
		Ent_DecodeSink
	} EntType;
	EntInfo() { };
	virtual ~EntInfo() { };
	int ent_id;
	int sf_id;
	int sess_id;
	EntInfo::EntType type;		// To be re-defined as enum?

	// Other data needed for an entity?
	int resx, resy;
};

typedef EntInfo* pEntInfo;
typedef map<int, pEntInfo> EntList;

class LocalItems
{
public:
	LocalItems();
	virtual ~LocalItems();
	void ClearAll(void) { sfList.clear(); sessList.clear(); entList.clear(); };

	int AddSession(int sessID);
	int AddSFToSession(int sfID, int sessID);
	int GetNumSFsInSession(int sessID) { return sessList[sessID] ? sessList[sessID]->session_in_these_sfs.size() : -1; };
	int RemoveSession(int sessID);
	int AddSourceToSession(int sessID, int sfID, int entID, const char* sourceName);
	int RemoveAllSourcesFromSession(int sessID);
	int RemoveSessionFromSF(int sessID, int sfID);
	void ListSessions(void);
	int ListSourcesInSession(int sessID);
	SessInfo* GetSessionInfo(int sessID);
	bool GetSFsForSession(int sessID, SFList& sfs, bool bEPOnly);

	int AddSFInternally(int sfID, const char* ip, int gID, const char* name, bool isEP);
	int AddSFLaunch(int sfID, const char* ip, int gID, const char* name);
	int RemoveSF(int sfID);
	void ListSFs(void);
	int GetNumSFs(void) { return sfList.size(); };

	int AddEntity(int entID, int sfID, int sessID, EntInfo::EntType type, int resx, int resy);
	int RemoveEntity(int entID);
	void ListEntities(void);
	EntInfo* GetEntInfo(int entID) { return entList[entID]; };

	map<string,EntInfo::EntType> entTypeMap;

protected:
	SFList sfList;
	SessList sessList;
	EntList entList;
	string inoutnull;
	string innull;
};

#endif /* LOCALITEMS_H_ */
