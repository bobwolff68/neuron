/*
 * localitems.cpp
 *
 *  Created on: Nov 29, 2010
 *      Author: rwolff
 */

#include "localitems.h"

LocalItems::LocalItems(const char* regsrvip)
{
    if (regsrvip)
        setRegServerPublicIP(regsrvip);
    else
        regServerPublic="NOT_SET_YET";

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
    SessionInfoOnSF.clear();
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
int LocalItems::AddSFToSession(int sfID, int sessID, int wanID, const char* sessname)
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

	if (pSess->SessionInfoOnSF[sfID])
		return DEST_SF_IN_USE;
	else
	{
	    pSessOnSFInfo psesssf;

        assert(sfList[sfID]);

	    psesssf = new SessOnSFInfo(sessID, sfList[sfID], wanID);
	    if (!psesssf)
	        return GENERIC_ERROR;

	    // Increment the count of how many sessions are inside an sf.
	    sfList[sfID]->num_sessions++;

	    // Now add it to the list.
	    pSess->SessionInfoOnSF[sfID] = psesssf;
	}

//	cout << "DEBUG: Exiting AddSFToSession " << endl
//			<< "- #factories=" << GetNumSFs() << endl;
//	cout << " Num SFs in Session (SessID=" << sessID << ") is:" << GetNumSFsInSession(sessID) << endl;
	return 0;
}

int LocalItems::RemoveSession(int sessID)
{
    pSessInfo pSess;

    pSess = sessList[sessID];

    if (!pSess)
            return ID_NOT_FOUND;

	// Remove all sources from the session before deleting the session.
	RemoveAllSourcesFromSession(sessID);

	// Now remove all session-per-sf info items.
	SessOnSFInfoList::iterator sit;

    for ( sit=pSess->SessionInfoOnSF.begin() ; sit != pSess->SessionInfoOnSF.end(); sit++ )
    {
        // Decrement number of sessions in the sf.
        assert(sfList[sit->first]);
        sfList[sit->first]->num_sessions--;

        // delete the entry in the list within the session
        assert(sit->second);
        delete sit->second;

        // Finally delete the entry itself and iterate again.
        pSess->SessionInfoOnSF.erase(sit);
    }

    assert(pSess->SessionInfoOnSF.size()==0);

    // Now we can actually delete the session info record.
	delete pSess;
	sessList.erase(sessID);

	cout << "Post-Session-Removal: List of Sessions..." << endl;
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
    pSessInfo pSess;

    pSess = sessList[sessID];
	if (!pSess)
		return ID_NOT_FOUND;

	if (pSess->SessionInfoOnSF[sfID])
	{
	    assert(sfList[sfID]);
	    sfList[sfID]->num_sessions--;

	    // delete the info entry object.
	    delete pSess->SessionInfoOnSF[sfID];

	    // Then remove the entry from the list.
		pSess->SessionInfoOnSF.erase(sfID);
	}
	else
		return DEST_SF_NOT_FOUND;

	return 0;
}

void LocalItems::ListSessions(void)
{
	  SessList::iterator sessit;
	  SFList::iterator sfit;
	  SessOnSFInfoList::iterator sessSFit;

	  cout << endl << "Number of Sessions currently: " << sessList.size() << endl;

	  // List all Sessions
	  for ( sessit=sessList.begin() ; sessit != sessList.end(); sessit++ )
	  {
	    cout << "Session ID=" << sessit->first;
	    if (sessit->second->sessName!="")
            cout << " with name='" << sessit->second->sessName << "'. is found in " << sessit->second->SessionInfoOnSF.size() << " Factories." << endl;
	    else
	        cout << ". Involved in " << sessit->second->SessionInfoOnSF.size() << " Factories." << endl;

	    // Now list those involved factories.
	    if (sessit->second->SessionInfoOnSF.size())
	    {
	        cout << "Details of each SF/Session status..." << endl;
	        cout << " SFID  State MediaWANID SF-Name" << endl;

	        assert (sessit->second);

            for ( sessSFit=sessit->second->SessionInfoOnSF.begin() ; sessSFit != sessit->second->SessionInfoOnSF.end(); sessSFit++ )
            {
                pSessOnSFInfo psInfo;
                psInfo = sessSFit->second;

                assert(psInfo);
                assert(psInfo->pSF);

                cout << sessSFit->first << "   " << (int)psInfo->state << "        " << psInfo->media_wan_id << "     " << psInfo->pSF->sf_name << endl;
            }
	    }

	  }
}

SessInfo* LocalItems::GetSessionInfo(int sessID)
{
	return sessList[sessID];
}

bool LocalItems::GetSFsForSession(int sessID, SFList& sfs, bool bEPOnly)
{
    pSessInfo pSess;

    pSess = sessList[sessID];
	if (!pSess)
	    return false;

	// Find the session first...
    // Found it. Iterate the SF list and return all SFs or only EP SFs if flagged.
	SessOnSFInfoList::iterator sessSFit;

    for (sessSFit=pSess->SessionInfoOnSF.begin() ; sessSFit != pSess->SessionInfoOnSF.end() ; sessSFit++ )
    {
        assert (sessSFit->second && sessSFit->second->pSF);

        if (!bEPOnly || (bEPOnly && sessSFit->second->pSF->is_endpoint))
            sfs[sessSFit->first] = sessSFit->second->pSF;
    }

	return true;
}

com::xvd::neuron::ObjectState LocalItems::GetCurStateInSFForSession(int sfid, int sessid)
{
    if (!sessList[sessid])
        return (com::xvd::neuron::ObjectState)DEST_SESS_NOT_FOUND;

    if (!sessList[sessid]->SessionInfoOnSF[sfid])
        return (com::xvd::neuron::ObjectState)DEST_SF_NOT_FOUND;

    // Make sure there is a current state registered for the sfid -- should be there.
    assert(sessList[sessid]->SessionInfoOnSF[sfid]);

    return sessList[sessid]->SessionInfoOnSF[sfid]->state;
};

int LocalItems::UpdateCurStateInSFForSession(int sfid, int sessid, com::xvd::neuron::ObjectState state)
{
    if (!sessList[sessid])
        return DEST_SESS_NOT_FOUND;

    if (!sessList[sessid]->SessionInfoOnSF[sfid])
        return DEST_SF_NOT_FOUND;

    // Make sure there is a current state registered for the sfid -- should be there.
    assert(sessList[sessid]->SessionInfoOnSF[sfid]);

    sessList[sessid]->SessionInfoOnSF[sfid]->state = state;

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

int LocalItems::AddSFLaunch(int sfID, const char* ip, const char* name, const char* usernameAt)
{
    string userString;

    userString = usernameAt;

	int ret;
	ret = AddSFInternally(sfID, ip, -1, -1, name, false);
	if (ret)
		return ret;

	if (userString!="")
	{
	    // Add trailing '@' if not present.
	    if (userString[userString.length()-1] != '@')
	        userString += "@";
	}

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
    stringstream extracmd;
#ifdef LOGSF_OUT
    // Always re-create the log file to start up clean.
    extracmd << " rm /tmp/sf_out" << sfID << ".log >/dev/null 2>&1;";
#endif

    sshnow << "ssh " << userString << ip << " \"source .bashrc;" << extracmd.str() << "./bin/sf " << sfID << " " << name << " 0 67 " << regServerPublic << " ";

#ifdef LOGSF_OUT
	sshnow << " </dev/null >>/tmp/sf_out" << sfID << ".log 2>&1 &\"";
#else
	sshnow << inoutnull + " &\"";
#endif
	cout << "This is the command: '" << sshnow.str() << "'" << endl;
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

    // Make sure the SF exists.
    if (!sfList[sfID])
        return DEST_SF_NOT_FOUND;

    if (!sessList[sessID])
        return DEST_SESS_NOT_FOUND;

    // Now ensure the session mentioned does exist on the sfid given.
    assert(sessList[sessID]->SessionInfoOnSF[sfID]);
    assert(sessList[sessID]->SessionInfoOnSF[sfID]->pSF == sfList[sfID]);

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

	// Add to the general list.
	entList[entID] = pEnt;

	// Add entity mapped by name ...
	entNameList[pEnt->entname] = pEnt;

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
	assert(pSess->SessionInfoOnSF[sfID]);
	pSF = pSess->SessionInfoOnSF[sfID]->pSF;

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

SessOnSFInfo::SessOnSFInfo(int sid, pSFInfo initpSF, int wID)
{
    assert(initpSF);
    sessid=sid;
    pSF=initpSF;
    state=com::xvd::neuron::OBJECT_STATE_INIT;
    media_wan_id=wID;
};
