/*
 * localitems.h
 *
 *  Created on: Nov 29, 2010
 *      Author: rwolff
 */

#ifndef LOCALITEMS_H_
#define LOCALITEMS_H_

#include "neuroncommon.h"
#include <sstream>
#include <map>

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
	string sf_ipaddress;
	string sf_name;
};

typedef SFInfo* pSFInfo;
typedef map<int, pSFInfo> SFList;

class SessInfo
{
public:
	SessInfo() { };
	virtual ~SessInfo() { };
	int sess_id;
	SFList session_in_these_sfs;
};

typedef SessInfo* pSessInfo;
typedef map<int, pSessInfo> SessList;

class EntInfo
{
public:
	EntInfo() { };
	virtual ~EntInfo() { };
	int ent_id;
	int sf_id;
	int sess_id;
	int type;		// To be re-defined as enum?
	// Other data needed for an entity?
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
	int AddSessionToSF(int sessID, int sfID);
	int RemoveSession(int sessID);
	int RemoveSessionFromSF(int sessID, int sfID);
	void ListSessions(void);

	int AddSF(int sfID, const char* ip, const char* name);
	int RemoveSF(int sfID);
	void ListSFs(void);

	int AddEntity(int entID, int sfID, int sessID, int type);
	int RemoveEntity(int entID);
	void ListEntities(void);

protected:
	SFList sfList;
	SessList sessList;
	EntList entList;
	string inoutnull;
	string innull;
};

#endif /* LOCALITEMS_H_ */
