/*
 * localitems.cpp
 *
 *  Created on: Nov 29, 2010
 *      Author: rwolff
 */

#include "localitems.h"

LocalItems::LocalItems()
{
	ClearAll();
	inoutnull = " </dev/null >/dev/null 2>&1 ";
	innull = " </dev/null ";
}

LocalItems::~LocalItems()
{
	  SFList::iterator sfit;
	  SessList::iterator sessit;
	  EntList::iterator entit;

	  // Delete all Entities
	  for ( entit=entList.begin() ; entit != entList.end(); entit++ )
	    delete entit->second;

	  // Delete all Sessions
	  for ( sessit=sessList.begin() ; sessit != sessList.end(); sessit++ )
	    delete sessit->second;

	  // Delete all SFs after asking each to 'die'
	  for ( sfit=sfList.begin() ; sfit != sfList.end(); sfit++ )
	    delete sfit->second;
}

int LocalItems::AddSession(int sessID)
{
	if (sessList[sessID])
		return ID_IN_USE;

	pSessInfo pEnt;

	pEnt = new SessInfo;
	if (!pEnt)
		return GENERIC_ERROR;

	pEnt->sess_id = sessID;
	pEnt->session_in_these_sfs.clear();

	sessList[sessID] = pEnt;

	cout << "List of Sessions..." << endl;
	ListSessions();

	return 0;
}

///
/// \brief Adds the sessID to the general sessList if it is not present.
///        Then adds the given sfID to the list of sf's involved in this session.
///
int LocalItems::AddSessionToSF(int sessID, int sfID)
{
	AddSession(sessID);

	pSessInfo pSess;

	pSess = sessList[sessID];
	if (!pSess)
		return ID_NOT_FOUND;

	assert(pSess->sess_id == sessID);

	if (pSess->session_in_these_sfs[sfID])
		return DEST_SF_IN_USE;
	else
		pSess->session_in_these_sfs[sfID] = sfList[sfID];

	return 0;
}

int LocalItems::RemoveSession(int sessID)
{
	if (!sessList[sessID])
		return ID_NOT_FOUND;

	delete sessList[sessID];
	sessList.erase(sessID);

	cout << "List of Sessions..." << endl;
	ListSessions();

	return 0;
}

///
/// \brief Remove this session from the sf_list in the session entry.
///        Does not remove the session from the sessList at all.
///
int LocalItems::RemoveSessionFromSF(int sessID, int sfID)
{
	if (!sessList[sessID])
		return ID_NOT_FOUND;

	if (sessList[sessID]->session_in_these_sfs[sfID])
		sessList[sessID]->session_in_these_sfs.erase(sfID);
	else
		return DEST_SF_NOT_FOUND;

	return 0;
}

void LocalItems::ListSessions(void)
{
	  SessList::iterator sessit;

	  cout << "Number of Sessions currently: " << sessList.size() << endl;

	  // List all Sessions
	  for ( sessit=sessList.begin() ; sessit != sessList.end(); sessit++ )
	    cout << "Session ID=" << sessit->first << ". Involved in " << sessit->second->session_in_these_sfs.size() << " Factories." << endl;
}

int LocalItems::AddSF(int sfID, const char* ip, const char* name)
{
	if (sfList[sfID])
		return ID_IN_USE;

	pSFInfo pSF;

	pSF = new SFInfo;
	if (!pSF)
		return GENERIC_ERROR;

	pSF->sf_id = sfID;
	pSF->sf_ipaddress = ip;
	pSF->sf_name = name;

	sfList[sfID] = pSF;

	// Now launch the remote 'sf'
	cout << "Launching remote 'sf' at " << ip << endl;
	string sshnow;
	sshnow = "ssh ";
	sshnow += ip;
	sshnow += " \"/home/rwolff/bin/sf -d ";
	sshnow += inoutnull + "\"";
//	cout << "This is the command: '" << sshnow << "'" << endl;
	system(sshnow.c_str());

	cout << "List of Factories..." << endl;
	ListSFs();

	return 0;
}

int LocalItems::RemoveSF(int sfID)
{
	if (!sfList[sfID])
		return ID_NOT_FOUND;

	delete sfList[sfID];
	sfList.erase(sfID);

	cout << "List of Factories..." << endl;
	ListSFs();

	return 0;
}

void LocalItems::ListSFs(void)
{
	  SFList::iterator sfit;

	  cout << "Number of Factories currently: " << sfList.size() << endl;

	  // List all SFs
	  for ( sfit=sfList.begin() ; sfit != sfList.end(); sfit++ )
	    cout << "Factory ID=" << sfit->first << " is on IP '" << sfit->second->sf_ipaddress << "'"
	    		<< " with name='" << sfit->second->sf_name << "'." << endl;
}

///
/// \brief  Adding an entity implies adding it to a particular place. This is slightly more
///         in depth for a "local" item. So, it must be added to the entList as well as being
///         added to the sessList's association list and the sf's list of sessions.
///
int LocalItems::AddEntity(int entID, int sfID, int sessID, int type)
{
	if (entList[entID])
		return ID_IN_USE;

	pEntInfo pEnt;

	pEnt = new EntInfo;
	if (!pEnt)
		return GENERIC_ERROR;

	pEnt->ent_id = entID;
	pEnt->sf_id = sfID;
	pEnt->sess_id = sessID;
	pEnt->type = type;

	// Make sure the SF exists.
	if (!sfList[sfID])
		return DEST_SF_NOT_FOUND;

	if (!sessList[sessID])
		return DEST_SESS_NOT_FOUND;

	// Add to the general list.
	entList[entID] = pEnt;

	// Now add this session to the associated SF.
	sessList[sessID]->session_in_these_sfs[sfID] = sfList[sfID];

	cout << "List of Entities..." << endl;
	ListEntities();

	return 0;
}

int LocalItems::RemoveEntity(int entID)
{
	if (!entList[entID])
		return ID_NOT_FOUND;

	delete entList[entID];
	entList.erase(entID);

	cout << "List of Entities..." << endl;
	ListEntities();

	return 0;
}

void LocalItems::ListEntities(void)
{
	  EntList::iterator entit;

	  cout << "Number of Entities currently: " << entList.size() << endl;

	  // List all Entities
	  for ( entit=entList.begin() ; entit != entList.end(); entit++ )
	    cout << "Entity ID=" << entit->first << " is found on sfID=" << entit->second->sf_id
	    		<< " and is in sessionID=" << entit->second->sess_id << "'." << endl;
}
