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

typedef struct SF_s {
	int sf_id;
	string sf_ipaddress;
} SFInfo, *pSFInfo;

typedef map<int, pSFInfo> SFList;

typedef struct Sess_s {
	int sess_id;
	SFList session_in_these_sfs;
} SessInfo, *pSessInfo;

typedef map<int, pSessInfo> SessList;

typedef struct Ent_s {
	int ent_id;
	int sf_id;
	int sess_id;
	int type;		// To be re-defined as enum?
	// Other data needed for an entity?
} EntInfo, *pEntInfo;

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

	int AddSF(int sfID, const char* ip);
	int RemoveSF(int sfID);
	void ListSFs(void);

	int AddEntity(int entID, int sfID, int sessID, int type);
	int RemoveEntity(int entID);
	void ListEntities(void);

protected:
	SFList sfList;
	SessList sessList;
	EntList entList;
};

#endif /* LOCALITEMS_H_ */
