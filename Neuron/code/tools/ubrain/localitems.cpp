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

	entTypeMap["NUMSOURCE"] = EntInfo::Ent_NumSource;
	entTypeMap["NUMSINK"]	= EntInfo::Ent_NumSink;
	entTypeMap["RP"]		= EntInfo::Ent_RP;
	entTypeMap["FILESOURCE"]= EntInfo::Ent_FileSource;
    entTypeMap["DECODESINK"]= EntInfo::Ent_DecodeSink;
    entTypeMap["SQSINK"]= EntInfo::Ent_SQSink;
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

SessInfo::SessInfo()
{
	session_in_these_sfs.clear();
	sourceList.clear();
}

SessInfo::~SessInfo()
{
	SourceInfoList::iterator sil;

	for (sil=sourceList.begin() ; sil != sourceList.end() ; sil++)
		delete sil->second;
}

int LocalItems::AddSession(int sessID, const char* sessname)
{
//	cout << "DEBUG: In Addsession with sessID=" << sessID << endl;
	if (sessList[sessID])
		return ID_IN_USE;

	pSessInfo pEnt;

	pEnt = new SessInfo;
	if (!pEnt)
		return GENERIC_ERROR;

	pEnt->sess_id = sessID;
	pEnt->sessName = sessname;

	sessList[sessID] = pEnt;
//	cout << "DEBUG: Exiting Addsession perfectly for sessID" << sessID << endl;

	return 0;
}

///
/// \brief Adds the sessID to the general sessList if it is not present.
///        Then adds the given sfID to the list of sf's involved in this session.
///
int LocalItems::AddSFToSession(int sfID, int sessID, const char* sessname)
{
//	cout << "DEBUG: In AddSFToSession " << endl
//			<< "- #factories=" << GetNumSFs() << endl;
//	cout << " Num SFs in Session (SessID=" << sessID << ") is:" << GetNumSFsInSession(sessID) << endl;
	AddSession(sessID, sessname);

	pSessInfo pSess;

	pSess = sessList[sessID];
	if (!pSess)
		return ID_NOT_FOUND;

	assert(pSess->sess_id == sessID);

	if (pSess->session_in_these_sfs[sfID])
		return DEST_SF_IN_USE;
	else
	{
		pSess->session_in_these_sfs[sfID] = sfList[sfID];
		pSess->curSessionStateOnSF[sfID] = com::xvd::neuron::OBJECT_STATE_INIT;
	}

//	cout << "DEBUG: Exiting AddSFToSession " << endl
//			<< "- #factories=" << GetNumSFs() << endl;
//	cout << " Num SFs in Session (SessID=" << sessID << ") is:" << GetNumSFsInSession(sessID) << endl;
	return 0;
}

int LocalItems::RemoveSession(int sessID)
{
	if (!sessList[sessID])
		return ID_NOT_FOUND;

	// Remove all sources from the session before deleting the session.

	delete sessList[sessID];
	sessList.erase(sessID);

	cout << "List of Sessions..." << endl;
	ListSessions();

	return 0;
}

int LocalItems::RemoveAllSourcesFromSession(int sessID)
{
	SessInfo* pSess = sessList[sessID];

	if (!pSess)
		return ID_NOT_FOUND;

	SourceInfoList::iterator sit;

	for ( sit=pSess->sourceList.begin() ; sit != pSess->sourceList.end(); sit++ )
	{
		assert(sit->second);
		delete sit->second;
		pSess->sourceList.erase(sit);
	}

	assert(pSess->sourceList.size()==0);

	return 0;
}

int LocalItems::ListSourcesInSession(int sessID)
{
	SessInfo* pSess = sessList[sessID];

	if (!pSess)
		return ID_NOT_FOUND;

	SourceInfoList::iterator sit;

	cout << "Number of sources in SessionID=" << sessID << " is:" << pSess->sourceList.size() << endl;

	for ( sit=pSess->sourceList.begin() ; sit != pSess->sourceList.end(); sit++ )
	{
		assert(sit->second);
		cout << "Source on " << sit->second->epName << ": " << sit->second->sourceName << endl;
	}

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
	  SFList::iterator sfit;

	  cout << endl << "Number of Sessions currently: " << sessList.size() << endl;

	  // List all Sessions
	  for ( sessit=sessList.begin() ; sessit != sessList.end(); sessit++ )
	  {
	    cout << "Session ID=" << sessit->first;
	    if (sessit->second->sessName!="")
            cout << " with name='" << sessit->second->sessName << "'. is found in " << sessit->second->session_in_these_sfs.size() << " Factories." << endl;
	    else
	        cout << ". Involved in " << sessit->second->session_in_these_sfs.size() << " Factories." << endl;

	    // Now list those involved factories.
	    if (sessit->second->session_in_these_sfs.size())
	    {
	        cout << "   SF ID's with session: ";
	        for ( sfit=sessit->second->session_in_these_sfs.begin() ; sfit != sessit->second->session_in_these_sfs.end(); sfit++ )
	        {
	            cout << sessit->first;      // Output the sfid.
	            // Now if there is an sfname attached, print it too. Else iterate.
	            if (sfit->second->sf_name!="")
	                cout << "/" << sfit->second->sf_name;

                cout << ", ";
	        }

	        cout << endl;
	    }

	  }
}

SessInfo* LocalItems::GetSessionInfo(int sessID)
{
	return sessList[sessID];
}

bool LocalItems::GetSFsForSession(int sessID, SFList& sfs, bool bEPOnly)
{
	SessList::iterator sessit;

	// Find the session first...
	for ( sessit=sessList.begin() ; sessit != sessList.end(); sessit++ )
	{
		if (sessit->first == sessID)
		{
			// Found it. Iterate the SF list and return all SFs or only EP SFs if flagged.
			SFList::iterator sfit;

			for (sfit=sessit->second->session_in_these_sfs.begin() ; sfit != sessit->second->session_in_these_sfs.end() ; sfit++ )
				if (bEPOnly || sfit->second->is_endpoint)
					sfs[sfit->first] = sfit->second;

			return true;
		}
	}

	return true;
}

com::xvd::neuron::ObjectState LocalItems::GetCurStateInSFForSession(int sfid, int sessid)
{
    if (!sessList[sessid])
        return (com::xvd::neuron::ObjectState)DEST_SESS_NOT_FOUND;

    if (!sessList[sessid]->session_in_these_sfs[sfid])
        return (com::xvd::neuron::ObjectState)DEST_SF_NOT_FOUND;

    // Make sure there is a current state registered for the sfid -- should be there.
    assert(sessList[sessid]->curSessionStateOnSF.find(sfid) != sessList[sessid]->curSessionStateOnSF.end());

    return sessList[sessid]->curSessionStateOnSF[sfid];
};

int LocalItems::UpdateCurStateInSFForSession(int sfid, int sessid, com::xvd::neuron::ObjectState state)
{
    if (!sessList[sessid])
        return DEST_SESS_NOT_FOUND;

    if (!sessList[sessid]->session_in_these_sfs[sfid])
        return DEST_SF_NOT_FOUND;

    // Make sure there is a current state registered for the sfid -- should be there.
    assert(sessList[sessid]->curSessionStateOnSF.find(sfid) != sessList[sessid]->curSessionStateOnSF.end());

    sessList[sessid]->curSessionStateOnSF[sfid] = state;

    return 0;
}

int LocalItems::AddSFInternally(int sfID, const char* ip, int acpID, int scpID, const char* name, bool isEP)
{
	if (sfList[sfID])
		return ID_IN_USE;

	pSFInfo pSF;

	pSF = new SFInfo;
	if (!pSF)
		return GENERIC_ERROR;

	pSF->sf_id = sfID;
    pSF->acp_slave_wan_id = acpID;
    pSF->scp_slave_wan_id = scpID;
	pSF->sf_ipaddress = ip;
	pSF->sf_name = name;
	pSF->is_endpoint = isEP;

	sfList[sfID] = pSF;

//	cout << "List of Factories..." << endl;
//	ListSFs();

	return 0;
}

//
// Use this flag to enable pseudo-logging of remote sf entities.
// WARNING: There **MUST** be a client reading the output log fifo at all times
//          via "tail -F /tmp/sf_out<sfid>.log"
//          Otherwise, the remote sf will block on its very first output (DDS coming online)
//
#define LOGSF_OUT

int LocalItems::AddSFLaunch(int sfID, const char* ip, const char* name)
{
	int ret;
	ret = AddSFInternally(sfID, ip, -1, -1, name, false);
	if (ret)
		return ret;

	//
	// TODO *MUST* Change commandline on 'sf' and get a REAL owner_id for the parent instead of '0'
	//

	string namecheck(name); //
	if (namecheck.find(" ") != string::npos)
	{
		cout << "Illegal name given. No spaces allowed." << endl;
		return GENERIC_ERROR;
	}

	// Now launch the remote 'sf'
	cout << "Launching remote Factory ID=" << sfID << " at " << ip << endl;
	stringstream sshnow;
	sshnow << "ssh " << ip << " \"source .bashrc;./bin/sf " << sfID << " " << name << " 0 67 " << ip << " ";
#ifdef LOGSF_OUT
	stringstream mcmd;
	// Always re-create the log file to start up clean.
	mcmd << "ssh " << ip << " \"rm /tmp/sf_out" << sfID << ".log >/dev/null 2>&1\"";

	system(mcmd.str().c_str());

	sshnow << " </dev/null >>/tmp/sf_out" << sfID << ".log 2>&1 &\"";
#else
	sshnow << inoutnull + " &\"";
//	sshnow << " </dev/null >sf_out" << sfID << ".log 2>&1 &\"";
#endif
//	cout << "This is the command: '" << sshnow.str() << "'" << endl;
	system(sshnow.str().c_str());

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

	  cout << endl << "Number of Factories currently: " << sfList.size() << endl;

	  // List all SFs
	  for ( sfit=sfList.begin() ; sfit != sfList.end(); sfit++ )
	  {
		  if (!sfit->second)
			  cout << "Factory ID=" << sfit->first << " ERROR-SF-NULL." << endl;
		  else
			  cout << "Factory ID=" << sfit->first << " is on IP '" << sfit->second->sf_ipaddress << "'"
	    		<< " with name='" << sfit->second->sf_name << "'." << endl;
	  }
}

///
/// \brief  Adding an entity implies adding it to a particular place. This is slightly more
///         in depth for a "local" item. So, it must be added to the entList as well as being
///         added to the sessList's association list and the sf's list of sessions.
///
int LocalItems::AddEntity(int entID, int sfID, int sessID, EntInfo::EntType type, const char* entname, int resx, int resy, int src_ent_id)
{
	if (entList[entID])
		return ID_IN_USE;

	pEntInfo pEnt;

	pEnt = new EntInfo;
	if (!pEnt)
		return GENERIC_ERROR;

	pEnt->ent_id = entID;
	pEnt->src_ent_id = src_ent_id;

	pEnt->sf_id = sfID;
	pEnt->sess_id = sessID;
	pEnt->type = type;
	pEnt->resx = resx;
	pEnt->resy = resy;
	pEnt->entname = entname;

	// Make sure the SF exists.
	if (!sfList[sfID])
		return DEST_SF_NOT_FOUND;

	if (!sessList[sessID])
		return DEST_SESS_NOT_FOUND;

	// Add to the general list.
	entList[entID] = pEnt;

	// Add entity mapped by name ...
	entNameList[pEnt->entname] = pEnt;

	// Now add this session to the associated SF.
	sessList[sessID]->session_in_these_sfs[sfID] = sfList[sfID];

	return 0;
}

int LocalItems::AddSourceToSession(int sessID, int sfID, int entID, const char* sourceName)
{
	pSessInfo pSess;
	pSFInfo pSF;
	pSourceInfo pSource;
	string epName;

	// Find the session - else all bets are off.
	pSess = sessList[sessID];

	if (!pSess)
		return DEST_SESS_NOT_FOUND;

	// Ensure the sf to which the source belongs is marked as "involved" with this session
	pSF = pSess->session_in_these_sfs[sfID];

	if (!pSF)
		return DEST_SF_NOT_FOUND;

	// Now create a Source and fill it in.
	pSource = new SourceInfo;
	if (!pSource)
		return GENERIC_ERROR;

	// Need to grab the SF's epName
	pSource->epName = pSF->sf_name;
	pSource->sourceName = sourceName;
	pSource->entid = entID;
	pSource->sessid = sessID;
	pSource->sfid = sfID;

	pSess->sourceList[entID] = pSource;

	return 0;
}

int LocalItems::RemoveEntity(int entID)
{
	if (!entList[entID])
		return ID_NOT_FOUND;

	if (!entNameList[entList[entID]->entname])
		return ID_NOT_FOUND;

	// Remove name from the entNameList
	entNameList.erase(entList[entID]->entname);

	// Now delete the entity and erase from the main list.
	delete entList[entID];
	entList.erase(entID);

	return 0;
}

void LocalItems::ListEntities(void)
{
	  EntList::iterator entit;

	  cout << endl << "Number of Entities currently: " << entList.size() << endl;

	  // List all Entities
	  for ( entit=entList.begin() ; entit != entList.end(); entit++ )
	  {
		  cout << "Entity ID=" << entit->first << flush;
		  if (!entit->second)
			  cout << " ERROR-ENTITY-NULL." << endl;
		  else
			  cout << " is found on sfID=" << entit->second->sf_id << " and is in sessionID=" << entit->second->sess_id << "'." << endl;
	  }
}
