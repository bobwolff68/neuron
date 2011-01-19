/*
 * ubrainmanager.cpp
 *
 *  Created on: Dec 3, 2010
 *      Author: rwolff
 */

#include "ubrainmanager.h"

uBrainManager::uBrainManager(int brainId, map<string,string> nvPairs, int domainId)
{
    local.setRegServerPublicIP(nvPairs["ubrain_ip"].c_str());

    DDSDomainParticipantFactory *factory =
            DDSDomainParticipantFactory::get_instance();

    DDS_DomainParticipantFactoryQos fqos;

    factory->get_qos(fqos);
    fqos.resource_limits.max_objects_per_thread = 8192;
    factory->set_qos(fqos);

    pCtrl = new Controller(brainId, domainId, nvPairs);

    pCtrl->SetCallback(this);
}

uBrainManager::~uBrainManager()
{
    if (pCtrl)
        delete pCtrl;
}

int uBrainManager::workerBee(void)
{
    return 0;
}

bool uBrainManager::GetUniqueSFidAndMarkSFAsCreated(int& sfIdOut)
{
    static int increasingSFID = 11555;

    // Here we only dole out a unique id number. We don't do anything with it until RegistrationComplete() is called.
    sfIdOut = increasingSFID++;

    cout << endl << "uBrainManager: Unique generated sfid=" << sfIdOut << endl;

    return true;
}

bool uBrainManager::GetUniqueEntityID(int& entIdOut)
{
    static int increasingEntID = 32768;

    entIdOut = increasingEntID++;
    cout << endl << "uBrainManager: Unique generated entid=" << entIdOut
            << endl;

    return true;
}

///
/// \brief When registration has been completed by RegServer::, it should tell the uBrainManager this fact.
///        The data presented allows the uBrainManager to finalize creation of the SF internally, matching
///        friendly name with sfid/ip address etc. The other crucial piece is the global ID for STUN.
///
bool uBrainManager::RegistrationComplete(map<string,string> pairs, bool isEP)
{
    string descr;
    string cmd, subcmd;
    map < string, string > nvP;
    int sfid;
    bool bok;

    if (isEP)
        sfid = FromString<int>(pairs["ep_sf_id"], bok);
    else
        sfid = FromString<int>(pairs["sfid"], bok);

    cout << "uBrainManager: RegComplete: sfid is: " << sfid << endl;

    //
    // Setup acp and scp wan id's in the controller for the newly added client/sf.
    //

    descr = "1@wan://::" + pairs["client_acp_id"];
    descr += ":1.1.1.1";

    cout << "INFO: RegistrationComplete: ACP PEER-add: " << descr << endl;

    if (!pCtrl->AddACPMasterPeer(descr.c_str()))
        cout << "ERROR: RegistrationComplete failed to add new SF as peer to Controller's ACP Master." << endl;

    descr = "1@wan://::" + pairs["client_scp_id"];
    descr += ":1.1.1.1";

    cout << "INFO: RegistrationComplete: SCP PEER-add: " << descr << endl;

    if (!pCtrl->AddSCPMasterPeer(descr.c_str()))
        cout << "ERROR: RegistrationComplete failed to add new SF as peer to Controller's SCP Master." << endl;
    cout << "Sleeping for 10 seconds..." << endl;
	usleep(10000000);

    // TODO - Fill in and correlate all registry data -- and create internal SF for endpoints.

    //
    // If the SF already exists, we added it -- it was not an endpoint.
    // So, make sure it's ready and also populate it's wan id's as they should be '-1' each currently.
    //
    pSFInfo psf;
    psf = local.GetSFInfo(sfid);
    if (psf)
    {
        assert(psf->is_endpoint == false);
	assert(!isEP);
        assert(psf->acp_slave_wan_id == -1);
        assert(psf->scp_slave_wan_id == -1);

        psf->acp_slave_wan_id = FromString<int>(pairs["client_acp_id"], bok);
        psf->scp_slave_wan_id = FromString<int>(pairs["client_scp_id"], bok);

        // Wait for new SF to become ready/online.
        nvP.clear();
        cmd = "SF";
        subcmd = "WAITFORSFREADY";
        nvP["sfid"] = ToString<int>(sfid);
        nvP["timeout"] = "25000";
        processDDS_SF(cmd, subcmd, nvP);

    }
    else
    {
//        assert(psf->is_endpoint == true);
//	assert(isEP);

        local.AddSFInternally(sfid, pairs["client_pub_ip"].c_str(),
                FromString<int>(pairs["client_acp_id"], bok), FromString<int>(pairs["client_scp_id"], bok),
                pairs["ep_friendly_name"].c_str(), isEP);

        // Wait for new SF to become ready/online.
        nvP.clear();
        cmd = "SF";
        subcmd = "WAITFORSFREADY";
        nvP["sfid"] = ToString<int>(sfid);
        nvP["timeout"] = "25000";
        processDDS_SF(cmd, subcmd, nvP);

        nvP.clear();
        // TODO MANJESH / RMW - This is hardwired to always add a new factory to session ID=1001
        nvP["sfid"] = ToString<int>(sfid);

        // TODO - Hardwired session id 1001 must be removed.
        nvP["sessid"] = ToString<int> (1001);

        cmd = "SF";
        subcmd = "ADDSESSION";
        processDDS_SF(cmd, subcmd, nvP);

        // Wait for new SF to become ready/online.
        nvP.clear();
        cmd = "SF";
        subcmd = "WAITFORSESSREADY";
        nvP["sfid"] = ToString<int>(sfid);
        // TODO - Hardwired session id 1001 must be removed.
        nvP["sessid"] = ToString<int> (1001);
        nvP["timeout"] = "25000";
        processDDS_SF(cmd, subcmd, nvP);
    }

    // TODO - Hardwired session id 1001 must be removed.
    RefreshSourcesOnSession(1001);

    // Now the remote Ep-SF exists...the controller will discover its existence automagically.

    return true;
}

bool uBrainManager::RefreshSourcesOnSession(int sessID)
{
    SFList sfl;
    pSessInfo pSess;
    stringstream sources;

    //
    // Get the list of SourceInfo for a session and iterate through it grabbing what we need from each.
    //
    pSess = local.GetSessionInfo(sessID);
    if (!pSess)
        return false;

    SourceInfoList::iterator siit;

    for (siit = pSess->sourceList.begin(); siit != pSess->sourceList.end() ; siit++)
    {
        // Need the SessionFactory epName (sf_name)
        if (pSess->session_in_these_sfs[siit->second->sfid])
        {
            assert(local.GetEntInfo(siit->second->entid));

            cout << "Source-Add: "
                    << pSess->session_in_these_sfs[siit->second->sfid]->sf_name
                    << " and SourceName=" << siit->second->sourceName << endl;

            sources << pSess->session_in_these_sfs[siit->second->sfid]->sf_name
                    << "/" << siit->second->sourceName << "~"
                    << siit->second->entid
            // Gotta get resx and resy from the EntInfo pointed to by second->entid
                    << "~" << local.GetEntInfo(siit->second->entid)->resx
                    << "~" << local.GetEntInfo(siit->second->entid)->resy
                    << ",";
        }
        else
            // ERROR - could not find the sf in the session - this is a BUG.
            sources << "ERROR_SF_NOT_FOUND_IN_SESSION" << "/"
                    << siit->second->sourceName << "~" << siit->second->entid
                    << ",";
    }

    string final;
    final = sources.str();

    // At the end, clip the final ',' if size is >0
    if (final.length())
        final.erase(final.length() - 1);

    cout << "DEBUG:Sources are:" << sources.str() << endl;
    //
    // Now put together the controller command to be sent to each SF involved which is an endpoint.
    //
    string cmd;

    if (!final.length())
        return true;

    // Now we finish up with sending the update when there are sources to send.
    cmd = FormulateScript("SENDSOURCES", final, sessID, 0);

    stringstream go;

    //
    // Now rip through the EP SFs involved in the session and send them each an update.
    //
    if (local.GetSFsForSession(sessID, sfl, true))
    {
        SFList::iterator sfit;

        for (sfit = sfl.begin(); sfit != sfl.end() ; sfit++)
        {
            // Must start fresh on each iteration.
            go.str("");

            // Now tell the controller about it.
            go << "scp update " << sessID << " " << sfit->first << " \"" << cmd
                    << "\"";
            cout << "Sending SrcList to SF=" << sfit->first << " '" << go.str()
                    << "'" << endl;

            if (!pCtrl->runSingleCommand(go.str().c_str()))
                return false;
        }
    }
    else
        return false;

    return true;
}

string uBrainManager::FormulateScript(const char* incmd, string& enttype,
        int entid, int ent_src, const char* optName)
{
    string cmd(incmd);
    stringstream scr;

    ToUpper<string>(cmd);

    if (cmd == "ADD")
    {
        scr << "add " << entid << " " << enttype;

        if (enttype == "natnumsrc")
            ; // All complete.
        else if (enttype == "stdoutsink")
            scr << " " << ent_src;
        else if (enttype == "vfsrc")
            scr << " " << optName;
        else if (enttype == "vdsink")
            scr << " " << ent_src;
        else if (enttype == "rp")
            scr << " " << ent_src;
        else if (enttype == "sqsink")
            scr << " " << ent_src;
        else
        {
            cout << "Error: Formulate - Illegal ADD of enttype=" << enttype
                    << endl;
            scr.str("ERROR_ENTTYPE_UNKNOWN");
            return scr.str();
        }
    }
    else if (cmd == "CHANGE")
    {
        scr << "updsrc " << entid << " " << ent_src;
    }
    else if (cmd == "DELETE")
    {
        scr << "rem " << entid;
    }
    else if (cmd == "SENDSOURCES")
    {
        //
        // ARGUMENT CHANGE/SUBSTITUTION FOR 'SENDSOURCES'
        //
        // The 'entid' in this case is actually the session id to find.
        //
        // The 'enttype' string is actually the string-argument for 'srclist []'
        //

        //		srclist [<username>/<srcname>~<srcEntityId>,]+
        scr << "srclist " + enttype;

    }

    return scr.str();
}

bool uBrainManager::ProcessDDS_SF_ChangeConnection(string& cmd, string& subcmd,
        map<string, string> & nvPairs)
{
    int sess_id;
    int sf_id;
    int ent_id;
    int src_ent_id;
    int resx, resy;
    bool isOK;
    stringstream go;

    string sfip = nvPairs["sfipaddress"];
    string sfname = nvPairs["sfname"];
    string entname = nvPairs["entname"];

    string enttype = nvPairs["enttype"];
    ToUpper<string>(enttype);

    sess_id = FromString<int> (nvPairs["sessid"], isOK);
    sf_id = FromString<int> (nvPairs["sfid"], isOK);
    ent_id = FromString<int> (nvPairs["entid"], isOK);

    // Used to designate the new connection to which the sink should subscribe
    src_ent_id = FromString<int> (nvPairs["srcentid"], isOK);

    assert(cmd == "SF");
    assert(subcmd == "CHANGECONNECTION");

    if (!requiredAttributesPresent(subcmd, nvPairs, "entid", "sessid", "sfid",
            "srcentid"))
    {
        cout
                << "ERROR: Required attribute(s) missing from 'sessid', 'entid', 'sfid', or 'srcentid'."
                << endl;
        return false;
    }

    string script;

    // Now prep to process the new connection.
    script = FormulateScript("CHANGE", enttype, ent_id, src_ent_id);
    // Now tell the controller about it.
    go << "scp update " << sess_id << " " << sf_id << " \"" << script << "\"";
    cout << "CHANGECONNECTION: Sending Controller: '" << go.str() << "'"
            << endl;

    if (!pCtrl->runSingleCommand(go.str().c_str()))
        return false;

    return true;
}

bool uBrainManager::ProcessDDS_SF_AddEntity(string& cmd, string& subcmd, map<
        string, string> & nvPairs)
{
    int sess_id;
    int sf_id;
    int ent_id;
    int src_ent_id;
    int resx, resy;
    bool isOK;
    stringstream go;

    string sfip = nvPairs["sfipaddress"];
    string sfname = nvPairs["sfname"];
    string entname = nvPairs["entname"];

    string enttype = nvPairs["enttype"];
    ToUpper<string>(enttype);

    sess_id = FromString<int> (nvPairs["sessid"], isOK);
    sf_id = FromString<int> (nvPairs["sfid"], isOK);
    ent_id = FromString<int> (nvPairs["entid"], isOK);
    src_ent_id = FromString<int> (nvPairs["srcentid"], isOK);
    resx = FromString<int> (nvPairs["resx"], isOK);
    resy = FromString<int> (nvPairs["resy"], isOK);

    assert(cmd == "SF");
    assert(subcmd == "ADDENTITY");

    if (!requiredAttributesPresent(subcmd, nvPairs, "entid", "sessid", "sfid",
            "enttype"))
    {
        cout
                << "ERROR: Required attribute(s) missing from 'sessid', 'entid', 'sfid', or 'enttype'."
                << endl;
        return false;
    }

    int ret;

    // Now process
    if (enttype == "NUMSOURCE" || enttype == "NUMSINK" || enttype
            == "FILESOURCE" || enttype == "DECODESINK" || enttype == "RP"
                    || enttype == "SQSINK")
    {
        if ((enttype == "DECODESINK" || enttype == "NUMSINK" || enttype == "RP" || enttype == "SQSINK")
                && !requiredAttributesPresent(subcmd, nvPairs, "srcentid"))
        {
            cout << "ERROR: Required attribute 'srcentid' missing." << endl;
            return false;
        }

/*
 * TODO - RMW-WAN
connect entid=2345 peer=wan:alsdkfas
connect entid=6789 peer=wan:dslafkjsdlk
connect sessid=sdf sfid=sdfs peer=wan:sadfsd
*/

        ret = local.AddEntity(ent_id, sf_id, sess_id,
                local.entTypeMap[enttype], entname.c_str(), resx, resy,
                src_ent_id);
        switch (ret)
        {
        case 0:
            // Success;
            break;
        case ID_IN_USE:
            cout << "Error: Entity ID=" << ent_id << " is already in use."
                    << endl;
            return false;
        case GENERIC_ERROR:
            cout << "Error: Memory or other generic problem adding Entity ID="
                    << ent_id << endl;
            return false;
        case DEST_SF_NOT_FOUND:
            cout << "Error: Factory ID=" << sf_id << " was not found." << endl;
            return false;
        case DEST_SESS_NOT_FOUND:
            cout << "Error: Session ID=" << sess_id << " was not found."
                    << endl;
            return false;
        default:
            cout << "Error: Other Error adding entity. Error value=" << ret
                    << endl;
            return false;
        }

        string script;
#if 0
        // Special case for numeric 'old' syntax where non-needed 'src' item needed to be the same as the ent_id.
        if (enttype=="NUMSOURCE")
        src_ent_id = ent_id;
#endif

        if (enttype == "NUMSOURCE")
            enttype = "natnumsrc";
        else if (enttype == "NUMSINK")
            enttype = "stdoutsink";
        else if (enttype == "FILESOURCE")
            enttype = "vfsrc";
        else if (enttype == "DECODESINK")
            enttype = "vdsink";
        else if (enttype == "RP")
            enttype = "rp";
        else if (enttype == "SQSINK")
            enttype = "sqsink";
        else
        {
            enttype = "ERROR_BAD_ENTTYPE:" + enttype;
            return false;
        }

        script = FormulateScript("ADD", enttype, ent_id, src_ent_id,
                entname.c_str());
        // Now tell the controller about it.
        go << "scp update " << sess_id << " " << sf_id << " \"" << script << " " << nvPairs["maxqueuelength"] << "\"";
        cout << "Sending Controller: '" << go.str() << "'" << endl;

        if (!pCtrl->runSingleCommand(go.str().c_str()))
            return false;

    }
    else
    {
        cout << "Error: ADDENTITY - Entity Type='" << enttype
                << "' is unknown at this time." << endl;
        return false;
    }

    return true;
}

bool uBrainManager::ProcessDDS_SF_DeleteEntity(string& cmd, string& subcmd,
        map<string, string> & nvPairs)
{
    int ent_id;
    int sf_id;
    int sess_id;
    bool isOK;
    stringstream go;

    ent_id = FromString<int> (nvPairs["entid"], isOK);

    assert(cmd.c_str() == "SF");
    assert(subcmd.c_str() == "DELETEENTITY");

    if (!requiredAttributesPresent(subcmd, nvPairs, "entid"))
    {
        cout << "ERROR: Required attribute(s) missing from 'entid'." << endl;
        return false;
    }

    // Before removing entity locally, find the session id and factory id associated with the entity.
    pEntInfo pEnt;
    pEnt = local.GetEntInfo(ent_id);
    if (!pEnt)
    {
        cout << "Error: DeleteEntity() Cannot find entity ID=" << ent_id
                << endl;
        return false;
    }

    sf_id = pEnt->sf_id;
    sess_id = pEnt->sess_id;

    int ret;

    ret = local.RemoveEntity(ent_id);
    switch (ret)
    {
    case 0:
        // Success;
        break;
    case ID_NOT_FOUND:
        cout << "Error: Entity ID=" << ent_id << " not found." << endl;
        return false;
    default:
        cout << "Error: Other Error deleting entity. Error value=" << ret
                << endl;
        return false;
    }

    string script;
    string enttype("none");
    script = FormulateScript("DELETE", enttype, ent_id, 0);
    // Now tell the controller about it.
    go << "scp update " << sess_id << " " << sf_id << " \"" << script << "\"";
    cout << "Sending Controller: '" << go.str() << "'" << endl;

    if (!pCtrl->runSingleCommand(go.str().c_str()))
        return false;

    return true;
}

bool uBrainManager::processDDS_SF(string& cmd, string& subcmd, map<string,
        string> & nvPairs)
{
    int sess_id;
    int sf_id;
    bool isOK;
    stringstream go;

    string sfip = nvPairs["sfipaddress"];
    string sfname = nvPairs["sfname"];

    sess_id = FromString<int> (nvPairs["sessid"], isOK);
    sf_id = FromString<int> (nvPairs["sfid"], isOK);

    assert(cmd == "SF");

#if 1
    if (subcmd == "TOFFER")
    {
        com::xvd::neuron::scp::State state;
        state.state = com::xvd::neuron::OBJECT_STATE_OFFERSRC;
        state.srcId = 5566;
        state.sessionId = 1001;
        state.payload = (char*) "3,MyFileSourceCamera-0,640,480";

        NewSessionState(&state);

        state.srcId = 5566;
        state.payload = (char*) "3,MyOtherFileSourceCamera-1,1024,768";

        NewSessionState(&state);

        state.srcId = 6000;
        state.payload = (char*) "3,THEIR_NEW_SourceCamera-1,1920,1080";

        NewSessionState(&state);
        return true;
    }
    else if (subcmd == "TREFRESH")
    {
        RefreshSourcesOnSession(1001);
        return true;
    }
    else if (subcmd == "TFIND")
    {
        if (!local.GetEntInfo(nvPairs["entname"].c_str()))
            cout << "Entname='" << nvPairs["entname"]
                    << "' not found in Entities list." << endl;
        else
        {
            EntInfo* pei;
            pei = local.GetEntInfo(nvPairs["entname"].c_str());
            cout << "Entname='" << nvPairs["entname"] << "' FOUND. ID is "
                    << pei->ent_id << endl;
        }
        return true;
    }
#endif

    if (subcmd == "ADDSESSION")
    {
        int ret;

        if (!requiredAttributesPresent(subcmd, nvPairs, "sfid", "sessid"))
        {
            cout
                    << "ERROR: Required attribute 'sfid' and/or 'sessid' not found."
                    << endl;
            return false;
        }

        ret = local.AddSFToSession(sf_id, sess_id, nvPairs["sessname"].c_str());

        switch (ret)
        {
        case 0:
            // All good. Now add via controller
            // Form command for controller
            go << "scp create " << sess_id << " " << sf_id;

            // Add the session name if attribute exists.
            if (nvPairs["sessname"] != "")
                go << " \"sessname " << nvPairs["sessname"] << "\"";

            cout << "Sending Controller: '" << go.str() << "'" << endl;

            if (!pCtrl->runSingleCommand(go.str().c_str()))
                return false;

            return true;
        case DEST_SF_IN_USE:
            cout << "Warning: SF ADDSESSION -- SFID=" << sf_id
                    << " already has SessionID=" << sess_id << endl;
            return true;
        case ID_NOT_FOUND:
            cout << "Error: SF ADDSESSION -- SessionID=" << sess_id
                    << " does not exist." << endl;
            return false;
        default:
            cout << "Error: SF ADDSESSION failed with error code=" << ret
                    << endl;
            return false;
        }

        assert(false);
    }
    else if (subcmd == "ADDENTITY")
    {
        return ProcessDDS_SF_AddEntity(cmd, subcmd, nvPairs);
    }
    else if (subcmd == "CHANGECONNECTION")
    {
        return ProcessDDS_SF_ChangeConnection(cmd, subcmd, nvPairs);
    }
    else if (subcmd == "DELETEENTITY")
    {
        return ProcessDDS_SF_DeleteEntity(cmd, subcmd, nvPairs);
    }
    else if (subcmd == "DELETESESSION")
    {
        if (!requiredAttributesPresent(subcmd, nvPairs, "sessid"))
        {
            cout << "ERROR: Required attribute 'sessid' not found." << endl;
            return false;
        }

        // Do the local removal and then kill it from the DDS controller.
        bool bok;
        local.RemoveSession(FromString<int>(nvPairs["sessid"], bok));

        // Form command for controller
        go << "scp delete " << sess_id;

        if (!pCtrl->runSingleCommand(go.str().c_str()))
            return false;

        return true;
    }
    else if (subcmd == "DELETEFACTORY")
    {
        if (!requiredAttributesPresent(subcmd, nvPairs, "sfid"))
        {
            cout << "ERROR: Required attribute 'sfid' not found." << endl;
            return false;
        }

        // Do the local removal and then kill it from the DDS controller.
        bool bok;
        local.RemoveSF(FromString<int>(nvPairs["sfid"], bok));

        // Form command for controller
        go << "acp shutdown " << sf_id;

        if (!pCtrl->runSingleCommand(go.str().c_str()))
            return false;

        return true;
    }
    else if (subcmd == "WAITFORSFREADY")
    {
        int ret;

        if (!requiredAttributesPresent(subcmd, nvPairs, "sfid", "timeout"))
        {
            cout << "ERROR: Required attribute 'sfid' and/or 'timeout'(ms) not found." << endl;
            return false;
        }

        cout << endl << "WAITING FOR SF " << nvPairs["sfid"] << " to become ready..." << endl;

        bool bok;
        ret = WaitForSFReady(FromString<int>(nvPairs["sfid"],bok), FromString<int>(nvPairs["timeout"], bok));
        switch (ret)
        {
        case 0:
            break;  // All is good.
        case TIMEOUT:
            // TODO - must deal with timeout via setting an error code which can be checked in language.
            cout << endl << endl << "***FAILURE*** -- *TIMEOUT* in WaitForSFReady command." << endl << endl;
            break;
        case ID_IN_USE:
            cout << endl << "**FAILURE** -- WaitForSFReady command - SF ID already being waited for." << endl << endl;
            break;
        default:
            cout << "***ERROR*** - WaitForSFReady command - Unknown return code=" << ret << endl;
            break;
        }
    }
    else if (subcmd == "WAITFORSESSREADY")
    {
        int ret;

        if (!requiredAttributesPresent(subcmd, nvPairs, "sfid", "sessid", "timeout"))
        {
            cout << "ERROR: Required attribute 'sfid' and/or 'sessid' and/or 'timeout'(ms) not found." << endl;
            return false;
        }

        cout << endl << "WAITING FOR SESSION=" << nvPairs["sessid"] << " on SF ID=" << nvPairs["sfid"] << " to become ready..." << endl;

        bool bok;
        ret = WaitForSessionReadyOnSF(FromString<int>(nvPairs["sessid"],bok), FromString<int>(nvPairs["sfid"],bok), FromString<int>(nvPairs["timeout"], bok));
        switch (ret)
        {
        case 0:
            break;  // All is good.
        case TIMEOUT:
            // TODO - must deal with timeout via setting an error code which can be checked in language.
            cout << endl << endl << "***FAILURE*** -- *TIMEOUT* in WaitForSessionReadyOnSF command." << endl << endl;
            break;
        case ID_IN_USE:
            cout << endl << "**FAILURE** -- WaitForSessionReadyOnSF command - SF ID already being waited for." << endl << endl;
            break;
        default:
            cout << "***ERROR*** - WaitForSessionReadyOnSF command - Unknown return code=" << ret << endl;
            break;
        }
    }
    else
    {
        cout << "Error: SF Command - Subcommand '" << subcmd << "' is unknown."
                << endl;
        return false;
    }

    return true;
}

bool uBrainManager::processDDSOriented(string& cmd, string& subcmd, map<string,
        string> & nvPairs)
{
    int sess_id;
    int sf_id;
    bool isOK;

    string sfip = nvPairs["sfipaddress"];
    string sfname = nvPairs["sfname"];

    sess_id = FromString<int> (nvPairs["sessid"], isOK);
    sf_id = FromString<int> (nvPairs["sfid"], isOK);

    if (cmd == "STATUS")
    {
        if (subcmd == "FACTORIES")
        {
            local.ListSFs();

            // Form command for controller
            stringstream go("acp ls");
            //			go << sfid;

            if (!pCtrl->runSingleCommand(go.str().c_str()))
                return false;
        }
        else if (subcmd == "SESSIONS")
        {
            local.ListSessions();

            // Form command for controller
            stringstream go("scp ls");
            //			go << sfid;

            if (!pCtrl->runSingleCommand(go.str().c_str()))
                return false;
        }
        else if (subcmd == "ENTITIES")
        {
            local.ListEntities();
        }
        else if (subcmd == "ENDPOINTS")
        {
            cout << "Status of endpoints not implemented." << endl;
        }
        else if (subcmd == "SOURCES")
        {
            if (!requiredAttributesPresent(subcmd, nvPairs, "sfipaddress"))
            {
                cout << "ERROR: Required attribute 'sfipaddress' not found."
                        << endl;
                return false;
            }

            // TODO - RMW - HARDWIRED? - iterate through all sessions .
            local.ListSourcesInSession(1001);
        }
        else
        {
            cout << "Unrecognized Command: " << cmd << " " << subcmd << endl;
            return false;
        }
    }
    else if (cmd == "CONNECT")
    {

    }
    else if (cmd == "SF")
    {
        processDDS_SF(cmd, subcmd, nvPairs);
    }
    else if (cmd == "SETUP")
    {

    }
    else
    {
        cout
                << "We should never arrive here - bad command in processDDSOriented()"
                << endl;
        assert(
                "We should never arrive here - bad command in processDDSOriented()"
                        == NULL);
        return false;
    }

    return true;
}

bool uBrainManager::processLocal(string& cmd, string& subcmd, map<string,
        string> & nvPairs)
{
    int sess_id;
    int sf_id;
    int ent_id;
    int resx, resy;
    bool isOK;

    string sfip = nvPairs["sfipaddress"];
    string sfname = nvPairs["sfname"];
    string sourcename = nvPairs["sourcename"];

    assert(cmd == "LOCAL");

    sess_id = FromString<int> (nvPairs["sessid"], isOK);
    sf_id = FromString<int> (nvPairs["sfid"], isOK);
    ent_id = FromString<int> (nvPairs["entid"], isOK);
    resx = FromString<int> (nvPairs["resx"], isOK);
    resy = FromString<int> (nvPairs["resy"], isOK);

    if (subcmd == "GENERATEKEYPAIR")
    {
        if (ssh.hasLocalKeypair())
        {
            cout << "We have a keypair. No need to generate." << endl;
            return false;
        }
        else
        {
            ssh.generateLocalKeypair();

            if (!ssh.hasLocalKeypair())
            {
                cout << "ERROR: Keypair generation FAILED. Exiting." << endl;
                return false;
            }
        }
    }
    else if (subcmd == "SENDPUBLICKEY" || subcmd == "TESTAUTHENTICATION")
    {
        if (!requiredAttributesPresent(subcmd, nvPairs, "sfipaddress"))
        {
            cout << "ERROR: Required attribute 'sfipaddress' not found."
                    << endl;
            return false;
        }

        if (subcmd == "SENDPUBLICKEY")
        {
            cout << " Pushing public key to remote server..." << endl;
            return ssh.pushLocalPublicKey(sfip);
        }
        else
        {
            cout
                    << " Testing secure connection and authentication with remote server..."
                    << endl;
            return ssh.testAuthentication(sfip);
        }

    }
    else if (subcmd == "CREATESESSION" || subcmd == "REMOVESESSION")
    {
        if (!requiredAttributesPresent(subcmd, nvPairs, "sessid"))
        {
            cout << "ERROR: Required attribute 'sessid' not found." << endl;
            return false;
        }

        if (subcmd == "CREATESESSION")
        {
            int ret;

            cout << "Creating session ID=" << sess_id << endl;
            ret = local.AddSession(sess_id, nvPairs["sessname"].c_str());
            if (ret)
            {
                switch (ret)
                {
                case ID_IN_USE:
                    cout << "ERROR: Session create of id=" << sess_id
                            << " failed. ID IN USE." << endl;
                    break;
                default:
                    cout << "ERROR: Session create of id=" << sess_id
                            << " failed with errno=" << ret << endl;
                    break;
                }
            }
            else
                cout << "Session create successful." << endl;
        }
        else
        {
            int ret;

            cout << "Deleting Session ID=" << sess_id << endl;
            ret = local.RemoveSession(sess_id);
            if (ret)
            {
                switch (ret)
                {
                case ID_NOT_FOUND:
                    cout << "ERROR: Removal of session id=" << sess_id
                            << " failed. ID NOT FOUND." << endl;
                    break;
                default:
                    cout << "ERROR: Removal of session id=" << sess_id
                            << " failed with errno=" << ret << endl;
                    break;
                }
            }
            else
                cout << "Session removal successful." << endl;
        }
    }
    else if (subcmd == "CREATESF" || subcmd == "REMOVESF")
    {
        if (subcmd == "CREATESF" && !requiredAttributesPresent(subcmd, nvPairs,
                "sfipaddress", "sfid"))
        {
            cout
                    << "ERROR: Required attributes 'sfipaddress' and/or 'sfid' not found."
                    << endl;
            return false;
        }

        if (subcmd == "REMOVESF" && !requiredAttributesPresent(subcmd, nvPairs,
                "sfid"))
        {
            cout << "ERROR: Required attribute 'sfid' not found." << endl;
            return false;
        }

        if (subcmd == "CREATESF")
        {
            int ret;

            cout << "Creating Factory ID=" << sf_id << endl;
            // For locally created SF's by the uBrain, we use the sf_id as the global stun id.
            // Only enpoints get other id's as they are external to the brain and could belong to another.
            ret = local.AddSFLaunch(sf_id, sfip.c_str(), sfname.c_str(), nvPairs["username"].c_str());
            if (ret)
            {
                switch (ret)
                {
                case ID_IN_USE:
                    cout << "ERROR: Factory create of id=" << sf_id
                            << " failed. ID IN USE." << endl;
                    break;
                case GENERIC_ERROR:
                    cout << "ERROR: Factory create of id=" << sf_id
                            << " failed. Likely call to ssh failed." << endl;
                    break;
                default:
                    cout << "ERROR: Factory create of id=" << sf_id
                            << " failed with errno=" << ret << endl;
                    break;
                }
            }
            else
                cout << "Factory create successful. Launch successful." << endl;
        }
        else // REMOVESF
        {
            int ret;

            cout << "Locally deleting Factory ID=" << sf_id << endl;

            ret = local.RemoveSF(sf_id);
            if (ret)
            {
                switch (ret)
                {
                case ID_NOT_FOUND:
                    cout << "ERROR: Removal of factory id=" << sf_id
                            << " failed. ID NOT FOUND." << endl;
                    break;
                default:
                    cout << "ERROR: Removal of factory id=" << sf_id
                            << " failed with errno=" << ret << endl;
                    break;
                }
            }
            else
                cout
                        << "Factory removal successful. (Remote launch was not killed via this command.)"
                        << endl;
        }
    }
    else if (subcmd == "KILLSF")
    {
        if (!requiredAttributesPresent(subcmd, nvPairs, "sfid"))
        {
            cout << "ERROR: Required attribute 'sfid' not found." << endl;
            return false;
        }

        // Now kill the remote and remove the factory afterwards.
        // TODO - need to implement an ssh with sed/awk to get the process id or send 'killall' ??
        //        This doesn't seem perfect by any means. Maybe it shouldn't be implemented.

        cout << "****" << endl
                << "  KILLSF is not implemented yet. Use REMOVESF for local only."
                << endl << "****" << endl;
        return false;
    }
    else if (subcmd == "CREATESOURCE")
    {
        int ret;

        if (!requiredAttributesPresent(subcmd, nvPairs, "sfid", "sessid",
                "entid", "sourcename"))
        {
            cout << "ERROR: Required attribute 'sfid', 'sessid', 'entid', or 'sourcename' not found." << endl;
            return false;
        }

        // Now the SourceInfo must be added to the session's list
        ret = local.AddSourceToSession(sess_id, sf_id, ent_id,
                sourcename.c_str());
        if (ret)
            cout << "Error: MANUALLY Adding source to session. Error ID="
                    << ret << endl;
    }
    else if (subcmd == "CONTROLLERDIRECT")
    {
        int ret;

        if (!requiredAttributesPresent(subcmd, nvPairs, "command"))
        {
            cout << "ERROR: Required attribute 'command' not found." << endl;
            return false;
        }

        // Now simply take the 'command' and send it to the controller.
        stringstream go;
        go << nvPairs["command"];
        cout << "Sending '" << nvPairs["command"] << "' directly to the controller." << endl;

        if (!pCtrl->runSingleCommand(go.str().c_str()))
            return false;
    }
    else
    {
        cout << "ERROR: LOCAL sub-command '" << subcmd << "' is unknown."
                << endl;
        return false;
    }

    // We're good if we just 'drop' to here.
    return true;
}

bool uBrainManager::requiredAttributesPresent(string& subcmd, map<string,
        string>& nvPairs, const char* attr1, const char* attr2,
        const char* attr3, const char* attr4, const char* attr5)
{
    // extract the destination server
    if (nvPairs.empty())
        return false;

    if (nvPairs[attr1] == "")
    {
        cout << subcmd << " Error. Requires attribute '" << attr1 << "'"
                << endl;
        return false;
    }

    if (attr2 != "" && nvPairs[attr2] == "")
    {
        cout << subcmd << " Error. Requires attribute '" << attr2 << "'"
                << endl;
        return false;
    }

    if (attr3 != "" && nvPairs[attr3] == "")
    {
        cout << subcmd << " Error. Requires attribute '" << attr3 << "'"
                << endl;
        return false;
    }

    if (attr4 != "" && nvPairs[attr4] == "")
    {
        cout << subcmd << " Error. Requires attribute '" << attr4 << "'"
                << endl;
        return false;
    }

    if (attr5 != "" && nvPairs[attr5] == "")
    {
        cout << subcmd << " Error. Requires attribute '" << attr5 << "'"
                << endl;
        return false;
    }

    return true;
}

void uBrainManager::ReceiveSelectSource(com::xvd::neuron::scp::State* state)
{
    int ret;
    stringstream payload;
    int sf_id;
    int ent_src_id, ent_sink_id;
    int sink_type;
    int sess_id;
    int resx, resy;
    char buf[100];
    string ent_type_string;

    payload.str(state->payload);
    sf_id = state->srcId;
    sess_id = state->sessionId;

    //
    // We will need to create the new entity remotely with a new id.
    //
    GetUniqueEntityID(ent_sink_id);

    //
    // Parse the inbound source select payload.
    //
    // <src_to_select_ent_id> <sink_type>
    //
    while (payload.peek() == ' ')
        payload.get();

    payload >> ent_src_id;

    while (payload.peek() == ',' || payload.peek() == ' ')
        payload.get();

    payload >> sink_type;

    //
    // Figure out where the source entity is and info about it for fill-in and hook-up.
    //
    pEntInfo pEnt;
    pEnt = local.GetEntInfo(ent_src_id);
    if (!pEnt)
    {
        assert(false);
        return;
    }

    EntInfo::EntType local_sink_ent_type;
    switch (sink_type)
    {
    case ENTITY_KIND_H264DECODERSINK:
        local_sink_ent_type = EntInfo::Ent_DecodeSink;
        ent_type_string = "DECODESINK";
        break;
    case ENTITY_KIND_STDOUTSINK:
        local_sink_ent_type = EntInfo::Ent_NumSink;
        ent_type_string = "NUMSINK";
        break;
    default:
        assert(false);
        break;
    }
    // Locally add the entity

    // Sanity check. State on input from the endpoint requesting the source should match these
    // values in the entity which is being selected
    assert(pEnt->sess_id == sess_id);

    //
    // Prep an addentity call and do it programmatically.
    //
    map < string, string > nvP;
    nvP["sfid"] = ToString<int> (sf_id);
    nvP["sessid"] = ToString<int> (sess_id);
    nvP["enttype"] = ent_type_string;
    nvP["entid"] = ToString<int> (ent_sink_id);
    nvP["srcentid"] = ToString<int> (ent_src_id);
    nvP["resx"] = ToString<int> (pEnt->resx);
    nvP["resy"] = ToString<int> (pEnt->resy);

    string cmd, subcmd;
    cmd = "SF";
    subcmd = "ADDENTITY";
    ProcessDDS_SF_AddEntity(cmd, subcmd, nvP);

}

void uBrainManager::ReceiveOfferSource(com::xvd::neuron::scp::State* state)
{
    int ret;
    stringstream payload;
    int sf_id;
    int sess_id;
    int ent_id;
    int ent_type;
    int resx, resy;
    char buf[100];
    string ent_type_string;

    payload.str(state->payload);
    sf_id = state->srcId;
    sess_id = state->sessionId;

    GetUniqueEntityID(ent_id);

    // Parse the inbound offer payload.
    while (payload.peek() == ' ')
        payload.get();

    // Source Offer format: <ent_type>,<sourceName>,res_x,res_y
    payload >> ent_type;
    while (payload.peek() == ',' || payload.peek() == ' ')
        payload.get();

    payload.getline(buf, 99, ',');
    while (payload.peek() == ',' || payload.peek() == ' ')
        payload.get();

    payload >> resx;
    while (payload.peek() == ',' || payload.peek() == ' ')
        payload.get();

    payload >> resy;

    EntInfo::EntType local_ent_type;

    switch (ent_type)
    {
    case ENTITY_KIND_H264FILESRC:
        local_ent_type = EntInfo::Ent_FileSource;
        ent_type_string = "FILESOURCE";
        break;
    case ENTITY_KIND_NATNUMSRC:
        local_ent_type = EntInfo::Ent_NumSource;
        ent_type_string = "NUMSOURCE";
        break;
    case ENTITY_KIND_H264DECODERSINK:
        local_ent_type = EntInfo::Ent_DecodeSink;
        ent_type_string = "DECODESINK";
        break;
    case ENTITY_KIND_STDOUTSINK:
        local_ent_type = EntInfo::Ent_NumSink;
        ent_type_string = "NUMSINK";
        break;
    default:
        assert(false);
        break;
    }

    // If session is not listed as part of the sf, then add it as it's obviously involved.
    assert(local.GetSessionInfo(sess_id));
    if (!local.GetSessionInfo(sess_id)->session_in_these_sfs[sf_id])
        local.AddSFToSession(sf_id, sess_id, "Auto(errant?)-Added-Session");

    // Now the SourceInfo must be added to the session's list
    ret = local.AddSourceToSession(sess_id, sf_id, ent_id, buf);
    if (ret)
        cout << "Error: Adding source to session. Error ID=" << ret << endl;

    //
    // Prep an addentity call and do it programmatically.
    //
    map < string, string > nvP;
    nvP["sfid"] = ToString<int> (sf_id);
    nvP["sessid"] = ToString<int> (sess_id);
    nvP["enttype"] = ent_type_string;
    nvP["entname"] = buf;
    nvP["entid"] = ToString<int> (ent_id);
    nvP["res"
            "x"] = ToString<int> (resx);
    nvP["resy"] = ToString<int> (resy);

    string cmd, subcmd;
    cmd = "SF";
    subcmd = "ADDENTITY";
    ProcessDDS_SF_AddEntity(cmd, subcmd, nvP);

    // Now send a refresh of all sources to all endpoints in the session.
    RefreshSourcesOnSession(sess_id);
}

///
/// \brief CallbackBase calls
///
void uBrainManager::NewSFDetected(int id)
{
//    cout << << "uBrainManager knows of a new SF ID=" << id << endl;
//            << endl << "Need IP, Name, GID to auto-populate in uBrain lists."
//            << endl << "****" << endl;
}

void uBrainManager::NewSFState(com::xvd::neuron::acp::State* state)
{
    /*

      com::xvd::neuron::OBJECT_STATE_INIT,
      com::xvd::neuron::OBJECT_STATE_STANDBY,
      com::xvd::neuron::OBJECT_STATE_READY,
      com::xvd::neuron::OBJECT_STATE_UPDATE,
      com::xvd::neuron::OBJECT_STATE_DELETE,
      com::xvd::neuron::OBJECT_STATE_DELETED,
    */

//    cout << "***New SF State for SFID=" << state->srcId << " with state->state = " << state->state << endl;

    // No matter what the state->state is...update the sf to which it belongs.
    if (!state->state == com::xvd::neuron::OBJECT_STATE_DELETE && !state->state == com::xvd::neuron::OBJECT_STATE_DELETED)
        assert(local.GetSFInfo(state->srcId) && "State update for non-existent SF??");

    if (local.GetSFInfo(state->srcId))
        local.UpdateCurSFState(state->srcId, state->state);

    switch (state->state)
    {
    case com::xvd::neuron::OBJECT_STATE_READY:
        break;
    default:
        break;
    }

}

void uBrainManager::NewSessionState(com::xvd::neuron::scp::State* state)
{
/*

  com::xvd::neuron::OBJECT_STATE_INIT,
  com::xvd::neuron::OBJECT_STATE_STANDBY,
  com::xvd::neuron::OBJECT_STATE_READY,
  com::xvd::neuron::OBJECT_STATE_UPDATE,
  com::xvd::neuron::OBJECT_STATE_DELETE,
  com::xvd::neuron::OBJECT_STATE_DELETED,
*/

//    cout << "***New Session State for SFID=" << state->srcId << " in Session=" << state->sessionId << " with state->state = " << state->state << endl;

    // No matter what the state->state is...update the session on the sf to which it belongs.
    if (!state->state == com::xvd::neuron::OBJECT_STATE_DELETE && !state->state == com::xvd::neuron::OBJECT_STATE_DELETED)
        assert(local.GetSessionInfo(state->sessionId) && local.GetSFInfo(state->srcId) && "State update for non-existent Session/SF??");

    if (local.GetSessionInfo(state->sessionId) && local.GetSFInfo(state->srcId))
        local.UpdateCurStateInSFForSession(state->srcId, state->sessionId, state->state);

    switch (state->state)
    {
    case com::xvd::neuron::OBJECT_STATE_OFFERSRC:
        ReceiveOfferSource(state);
        break;
    case com::xvd::neuron::OBJECT_STATE_SELECTSRC:
        cout << "INFO: Source selected...do something with it - hook 'em up." << endl;
        ReceiveSelectSource(state);
        break;
    case com::xvd::neuron::OBJECT_STATE_READY:
//        cout << "New Session State: Session ID=" << state->sessionId << ", SF ID==" << state->srcId << " was found to be READY." << endl;

        assert(local.GetSessionInfo(state->sessionId));
        break;
    default:
        break;
    }

}

int uBrainManager::WaitForSFReady(int sfid, int timeInms)
{
    const int usInterval = 100000;
    int waited = 0;

    if (!local.GetSFInfo(sfid))
        return ID_NOT_FOUND;

    // Now it's inserted, simply wait for the prescribed time in 100ms increments until valid or timeout
    while (waited < timeInms * 1000)
    {
        if (local.GetCurSFState(sfid)==com::xvd::neuron::OBJECT_STATE_READY)
            return 0;

        usleep(usInterval);

        waited += usInterval;
    }

    // Timeout occured.
    cout << "WaitingForSFReady: Timeout." << endl;
    // TODO need mutex around usage.

    return TIMEOUT;
}

int uBrainManager::WaitForSessionReadyOnSF(int sessid, int sfid, int timeInms)
{
    const int usInterval = 100000;
    int waited = 0;

    if (!local.GetSessionInfo(sessid))
        return DEST_SESS_NOT_FOUND;

    if (!local.GetSFInfo(sfid))
        return DEST_SF_NOT_FOUND;

    // Now simply wait for the prescribed time in 100ms increments until valid or timeout
    while (waited < timeInms * 1000)
    {
        if (local.GetCurStateInSFForSession(sfid, sessid)==com::xvd::neuron::OBJECT_STATE_READY)
            return 0;

        usleep(usInterval);

        waited += usInterval;
    }

    // Timeout occured.
    cout << "WaitingForSessionReadyOnSF: Timeout." << endl;

    return TIMEOUT;
}
