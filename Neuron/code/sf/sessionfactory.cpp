#include <iostream>
#include <unistd.h>
#include "sessionfactory.h"

//-------------------------------- LOCAL SCOPE FUNCTIONS -------------------------------------------
bool GetPeerDescListFromPeerList(string &Value,map<int,string> &MediaPeerDescList)
{
    char            buf[100];
    bool            isOk;
    int             peerWanId;
    string          PeerMaxPartIdxStr;
    string          PeerWanIdStr;
    stringstream    liststream;
    
    
    liststream << Value;
    while(liststream.good())
    {
        string  WanDescriptor = "[";
        
        liststream.getline(buf,99,'~');
        PeerWanIdStr = buf;
        peerWanId = FromString<int>(PeerWanIdStr,isOk);
        if(!liststream.good())    return false;
        if(!isOk)   return false;
        
        liststream.getline(buf,99,'~');
        PeerMaxPartIdxStr = buf;
        FromString<int>(PeerMaxPartIdxStr,isOk);
        if(!isOk)   return false;

	    //Convert Peer Wan ID to hex.
	    stringstream int2hexstream;
	    int2hexstream << hex << FromString<int>(PeerWanIdStr,isOk);
	    if(!isOk)   return false;

        WanDescriptor += (PeerMaxPartIdxStr+"]@wan://::");
        WanDescriptor += (int2hexstream.str()+":1.1.1.1");
        MediaPeerDescList[peerWanId] = WanDescriptor;
        cout << "Descriptor: " << WanDescriptor << endl;
    }
    
    return true;
}


//-------------------------------- SESSION FACTORY MEMBERS -----------------------------------------
SessionFactory::SessionFactory(IDType sfIdParam,const char *nameParam,IDType ownerIdParam,
							   int domId,map<string,string> &nvPairs)
: EventHandlerT<SessionFactory>(),ThreadSingle()
{
	char	SCPSlaveName[100];
	char	ACPSlaveName[100];
	char	LSCPMasterName[100];
    string  QosProfileACP = "ACPLAN";
    string  QosProfileSCP = "SCPLAN";

    map<string,string>      PropertyPairsACP;
    map<string,DDS_Boolean> PropagateDiscoveryFlagsACP;
    map<string,string>      PropertyPairsSCP;
    map<string,DDS_Boolean> PropagateDiscoveryFlagsSCP;
    map<string,string>      PropertyPairsLSCP;
    map<string,DDS_Boolean> PropagateDiscoveryFlagsLSCP;
    
	id = sfIdParam;
	stop = false;
	this->domId = domId;
	strcpy(name,nameParam);
	ownerId = ownerIdParam;
	StunLocator = "";
	lanOnlyMode = true;
	
	if(nvPairs["use_lan_only"]!="true")
	{
        StunLocator = nvPairs["stun_ip"];
        QosProfileACP = "ACP";
        QosProfileSCP = "SCP";
        lanOnlyMode = false;
    }        

// Set to WARNING status messages
//    NDDSConfigLogger::get_instance()->
//        set_verbosity_by_category(NDDS_CONFIG_LOG_CATEGORY_API,
//                                  NDDS_CONFIG_LOG_VERBOSITY_WARNING);

//If debug dds libs used, set verbosity high
//    NDDSConfigLogger::get_instance()->
//        set_verbosity_by_category(NDDS_CONFIG_LOG_CATEGORY_API,
//                                  NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);

	// Create Admin Control Plane (ACP) slave
	GEN_CP_INTERFACE_NAME(ACPSlaveName,name,ACP_SLAVE_NAME);
	if(!lanOnlyMode)
	{
	    PropertyPairsACP["dds.transport.wan_plugin.wan.transport_instance_id"] = nvPairs["client_acp_id"];
	    PropagateDiscoveryFlagsACP["dds.transport.wan_plugin.wan.transport_instance_id"] = DDS_BOOLEAN_FALSE;
	    PropertyPairsACP["dds.transport.wan_plugin.wan.server"] = nvPairs["stun_ip"];
	    PropagateDiscoveryFlagsACP["dds.transport.wan_plugin.wan.server"] = DDS_BOOLEAN_FALSE;
	}
	PropertyPairsACP["CPInterfaceType"] = "ACP:Slave";
	PropagateDiscoveryFlagsACP["CPInterfaceType"] = DDS_BOOLEAN_TRUE;	
	PropertyPairsACP["Id"] = ToString<int>(id);
	PropagateDiscoveryFlagsACP["Id"] = DDS_BOOLEAN_TRUE;	
	PropertyPairsACP["MasterId"] = ToString<int>(ownerId);
	PropagateDiscoveryFlagsACP["MasterId"] = DDS_BOOLEAN_TRUE;	
	pACSlave = new ACPSlave(this,id,domId,ACPSlaveName,PropertyPairsACP,PropagateDiscoveryFlagsACP,QosProfileACP.c_str());
	if(!lanOnlyMode)
    	pACSlave->AddPeerAndWaitForDiscovery(nvPairs["ubrain_acp_desc"].c_str(),5000);
	pACSlaveObj = pACSlave->CreateSlaveObject(id);

	// Create Session Control Plane (SCO) slave
	GEN_CP_INTERFACE_NAME(SCPSlaveName,name,SCP_SLAVE_NAME);

	if(!lanOnlyMode)
	{	
	    PropertyPairsSCP["dds.transport.wan_plugin.wan.transport_instance_id"] = nvPairs["client_scp_id"];
	    PropagateDiscoveryFlagsSCP["dds.transport.wan_plugin.wan.transport_instance_id"] = DDS_BOOLEAN_FALSE;
	    PropertyPairsSCP["dds.transport.wan_plugin.wan.server"] = nvPairs["stun_ip"];
	    PropagateDiscoveryFlagsSCP["dds.transport.wan_plugin.wan.server"] = DDS_BOOLEAN_FALSE;
    }
	PropertyPairsSCP["CPInterfaceType"] = "SCP:Slave";
	PropagateDiscoveryFlagsSCP["CPInterfaceType"] = DDS_BOOLEAN_TRUE;	
	PropertyPairsSCP["Id"] = ToString<int>(id);
	PropagateDiscoveryFlagsSCP["Id"] = DDS_BOOLEAN_TRUE;	
	PropertyPairsSCP["MasterId"] = ToString<int>(ownerId);
	PropagateDiscoveryFlagsSCP["MasterId"] = DDS_BOOLEAN_TRUE;	
	pSCSlave = new SCPSlave(this,id,domId,SCPSlaveName,PropertyPairsSCP,PropagateDiscoveryFlagsSCP,QosProfileSCP.c_str());
	if(!lanOnlyMode)
        pSCSlave->AddPeerAndWaitForDiscovery(nvPairs["ubrain_scp_desc"].c_str(),5000);

	// Create Local Session Control Plane (LSCP) master
	GEN_CP_INTERFACE_NAME(LSCPMasterName,name,LSCP_MASTER_NAME);
	PropertyPairsLSCP["CPInterfaceType"] = "LSCP:Master";
	PropagateDiscoveryFlagsLSCP["CPInterfaceType"] = DDS_BOOLEAN_TRUE;	
	PropertyPairsLSCP["Id"] = ToString<int>(id);
	PropagateDiscoveryFlagsLSCP["Id"] = DDS_BOOLEAN_TRUE;	
	pLSCMaster = new LSCPMaster(this,id,domId,LSCPMasterName,PropertyPairsLSCP,PropagateDiscoveryFlagsLSCP,"LSCP");

	// Initialize ACP Instances
	acControl = com::xvd::neuron::acp::ControlTypeSupport::create_data();
	acState = com::xvd::neuron::acp::StateTypeSupport::create_data();
	acEvent = com::xvd::neuron::acp::EventTypeSupport::create_data();
	acMetrics = com::xvd::neuron::acp::MetricsTypeSupport::create_data();

	// Generate hash table of Event Handle Function Ptrs with Event kind as key
	AddHandleFunc(&SessionFactory::HandleNewSessionEvent,SCP_EVENT_NEW_SESSION);
	AddHandleFunc(&SessionFactory::HandleUpdateSessionEvent,SCP_EVENT_UPDATE_SESSION);
	AddHandleFunc(&SessionFactory::HandleDeleteSessionEvent,SCP_EVENT_DELETE_SESSION);
	AddHandleFunc(&SessionFactory::HandleSessionUpdateStateEvent,LSCP_EVENT_SESSION_STATE_UPDATE);
	AddHandleFunc(&SessionFactory::HandleACPNewSessionEvent,ACP_EVENT_NEW_SESSION);
	AddHandleFunc(&SessionFactory::HandleACPUpdateSessionEvent,ACP_EVENT_UPDATE_SESSION);
	AddHandleFunc(&SessionFactory::HandleACPDeleteSessionEvent,ACP_EVENT_DELETE_SESSION);

	// Create Resource Monitor Object
	//TRY_NEW(pResMtrObj,ResourceMonitor,"SessionFactory::SessionFactory()/new(ResourceMonitor): ");

    partIdxAvailable = 3;
	SetStateStandby();
}

SessionFactory::~SessionFactory()
{
	//Delete all sessions
	map<int,RemoteSessionSF *>::iterator	it;
	for(it=SessionList.begin(); it!=SessionList.end(); it++)
	{
		std::cout << "Delete" << endl;
		delete it->second;
	}

	SessionList.clear();
	delete pLSCMaster;
	delete pSCSlave;
	SetStateDeleted();
	pACSlave->DeleteSlaveObject(pACSlaveObj);
	delete pACSlave;
	DDSDomainParticipantFactory::finalize_instance();
}

void SessionFactory::HandleNewSessionEvent(Event *pEvent)
{
	RemoteSessionSF		*pSession = NULL;
	SCPEventNewSession 	*pNewSessEvt = NULL;
	SCPSlaveObject		*pSCSlaveObjLocal = NULL;

	if(pEvent!=NULL)
	{
		pNewSessEvt = reinterpret_cast<SCPEventNewSession *>(pEvent);
		pSCSlaveObjLocal = (SCPSlaveObject *)(pNewSessEvt->GetSession());

		if(pSCSlaveObjLocal!=NULL)
		{
			std::cout << "srcId: " << pSCSlaveObjLocal->GetSrcId() << endl;
			if(pSCSlaveObjLocal->GetSrcId()==id)
			{
				if(SessionList.find(pSCSlaveObjLocal->GetSessionId())!=SessionList.end())
				{
					// Error, session already present
					std::cout << SF_LOG_PROMPT(id) << "(NEWSESSION): Session " <<
						pSession->GetId() << " already present" << endl;

                    //TODO: Send already present message back to brain
				}
				else
				{
					// Creating a new RemoteSessionSF Object will create an SL for that session
					string                  SessionName = "";
					map<string,string>      PropertyPairsMedia;
                    map<string,DDS_Boolean> PropagateDiscoveryFlagsMedia;
                    map<int,string>         PeerDescListMedia;
                    
                    if(
                        ProcessScriptOnNewSession((const char *)pNewSessEvt->GetData()->script,
                                                  SessionName,PropertyPairsMedia,
                                                  PropagateDiscoveryFlagsMedia,
                                                  PeerDescListMedia)
                      )
                    {
                        if(SessionName=="")
                        {
                            SessionName = "SF(";
					        SessionName += (ToString<int>(id)+")=>SL(");
					        SessionName += (ToString<int>(pSCSlaveObjLocal->GetSessionId())+")");
    					    cout << "Session name unspecified, using default: "
    					         << SessionName << endl;
                        }
                        
                        if(!lanOnlyMode)
                        {
                            PropertyPairsMedia["dds.transport.wan_plugin.wan.server"] = StunLocator;
                            PropagateDiscoveryFlagsMedia["dds.transport.wan_plugin.wan.server"] = DDS_BOOLEAN_FALSE;
                            PropertyPairsMedia["participant_id"] = ToString<int>(++partIdxAvailable);
                            partIdxAvailable++;
                        }
                        
                        pSession = new RemoteSessionSF(pSCSlave,pSCSlaveObjLocal,pLSCMaster,
                                                       domId,id,lanOnlyMode,SessionName.c_str(),
                                                       PropertyPairsMedia,
                                                       PropagateDiscoveryFlagsMedia,
                                                       PeerDescListMedia,NEW_SL_THREAD);
					    
					    SessionList[pSession->GetId()] = pSession;
					    std::cout << SF_LOG_PROMPT(id) << ": New Session(" << pSession->GetId() <<
						      ")" << endl;
                    }
                    else
                        cout << "Error processing session init script, aborting..." << endl;
				}
			}
		}
		else
			std::cout << SF_LOG_PROMPT(id) <<
				": (New Session): Can't handle null SCPSlaveObject" << endl;
	}
	else
		std::cout << SF_LOG_PROMPT(id) << ": (New Session): Can't handle null event" << endl;

	return;
}

void SessionFactory::HandleUpdateSessionEvent(Event *pEvent)
{
	RemoteSessionSF			*pSession = NULL;
	SCPEventUpdateSession	*pUpSessEvt = NULL;

	//Get the session object for the requested session id
	if(pEvent!=NULL)
	{
		pUpSessEvt = reinterpret_cast<SCPEventUpdateSession *>(pEvent);

		std::cout << "srcId: " << pUpSessEvt->GetData()->srcId << endl;
		if(pUpSessEvt->GetData()->srcId==ownerId)
		{
			if(SessionList.find(pUpSessEvt->GetData()->sessionId)==SessionList.end())
			{
				//Error, session not found
				std::cout << SF_LOG_PROMPT(id) << "(UPSESSION): Session " <<
					pUpSessEvt->GetData()->sessionId << " not found" << endl;

                //TODO: Send not found message to ubrain
			}
			else
			{
				pSession = SessionList[pUpSessEvt->GetData()->sessionId];
				pSession->SetStateUpdate();
				pSession->Update(pUpSessEvt->GetData());
				std::cout << SF_LOG_PROMPT(id) << ": Session Update(" << pSession->GetId() << "): "
					<< pUpSessEvt->GetData()->script << endl;
			}
		}
	}
	else
		std::cout << SF_LOG_PROMPT(id) << ": (Update Session): Can't handle null event" << endl;

	return;
}

void SessionFactory::HandleDeleteSessionEvent(Event *pEvent)
{
	RemoteSessionSF			*pSession = NULL;
	SCPEventDeleteSession	*pDelSessEvt = NULL;

	if(pEvent!=NULL)
	{
		pDelSessEvt = reinterpret_cast<SCPEventDeleteSession *>(pEvent);
		if(SessionList.find(pDelSessEvt->GetSessionId())==SessionList.end())
		{
			//Error, session not found
			std::cout << SF_LOG_PROMPT(id) << "(DELSESSION): Session " <<
				pDelSessEvt->GetSessionId() << " not found" << endl;

            //TODO: Send not found message to ubrain
		}
		else
		{
			pSession = SessionList[pDelSessEvt->GetSessionId()];
			SessionList.erase(pDelSessEvt->GetSessionId());
			delete pSession;
			std::cout << SF_LOG_PROMPT(id) <<
				": Delete Session(" << pDelSessEvt->GetSessionId() << ")" << endl;
		}
	}
	else
		std::cout << SF_LOG_PROMPT(id) << ": (Delete Session): Can't handle null event" << endl;

	return;
}


void SessionFactory::HandleSessionUpdateStateEvent(Event *pEvent)
{
	RemoteSessionSF				   *pSession = NULL;
	LSCPEventSessionStateUpdate	   *pUpStateEvt = NULL;
	com::xvd::neuron::lscp::State  *lscState = NULL;

	if(pEvent!=NULL)
	{
		pUpStateEvt = reinterpret_cast<LSCPEventSessionStateUpdate*>(pEvent);
		lscState = pUpStateEvt->GetData();

		if(lscState->srcId==id)
		{
			if(SessionList.find(lscState->sessionId)==SessionList.end())
			{
				//Error, session not found
				std::cout << SF_LOG_PROMPT(id) << "(UPSTSESSION): Session " << lscState->sessionId <<
					" not found" << endl;
			}
			else
			{
				pSession = SessionList[lscState->sessionId];
				pSession->HandleSLState(lscState);
			}
		}
		else
            std::cout << "Unexpected state: sfid=" << id << ", slsrcid=" << lscState->srcId
                      << "state: " << lscState->state << std::endl;
	}
	else
		std::cout << SF_LOG_PROMPT(id) << ": (Update State): Can't handle null event" << endl;

	return;
}

void SessionFactory::HandleACPNewSessionEvent(Event *pEvent)
{
    //TODO: Check to see if event is generated by its owner ubrain
	SetStateReady();

	return;
}

void SessionFactory::HandleACPUpdateSessionEvent(Event *pEvent)
{
	std::cout << SF_LOG_PROMPT(id) << ": ACP Update --> " <<
		reinterpret_cast<ACPEventUpdateSession*>(pEvent)->GetData()->script << endl;

	return;
}

void SessionFactory::HandleACPDeleteSessionEvent(Event *pEvent)
{
    //TODO: Check to see if event is generated by its owner ubrain
	stop = true;
	return;
}

void SessionFactory::EventHandleLoop(void)
{
	while(!isStopRequested)
		HandleNextEvent();

	while(!NoEvents())
		HandleNextEvent();

	return;
}

bool SessionFactory::ProcessScriptOnNewSession(const char *script,string &SessionName,
                                               map<string,string> &PropertyPairs,
                                               map<string,DDS_Boolean> &PropagateDiscoveryFlags,
                                               map<int,string> &MediaPeerDescList)
{
    string          Name;
    string          Value;
    stringstream    scriptstream;
    
    scriptstream << script;
    cout << "New session init script: " << script << endl;

    // Kill leading double-quote
    if (scriptstream.peek()=='"')
    	scriptstream.get();
    
    while(scriptstream.good())
    {
        scriptstream >> Name;
	if (!scriptstream.good())
		return false;

        scriptstream >> Value;
        
	// Look out for trailing double-quote in value.
	if (Value.rfind('"')!=string::npos)
		Value = Value.substr(0, Value.length()-1);

        if(Name=="sessname")
        {    
            SessionName = Value;
            cout << "SessionName: " << Value << endl;
        }
        else if(Name=="mediawanid")
        {
            if(!lanOnlyMode)
            {
                PropertyPairs["dds.transport.wan_plugin.wan.transport_instance_id"] = Value;
	            PropagateDiscoveryFlags["dds.transport.wan_plugin.wan.transport_instance_id"] = DDS_BOOLEAN_FALSE;
	            cout << "MediaWanId: " << Value << endl;
	        }
	    }
	    else if(Name=="mediapeers")
        {	
            if(!lanOnlyMode)
            {
                if(!GetPeerDescListFromPeerList(Value,MediaPeerDescList))
	                return false;
	        }
	    }
   }
   
   return true;
}

