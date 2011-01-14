#include <iostream>
#include <new>
#include <unistd.h>
#include "sessionfactory.h"

SessionFactory::SessionFactory(IDType sfIdParam,const char *nameParam,IDType ownerIdParam,
							   int domId,const char *scpSlaveWanIdStr,const char *acpSlaveWanIdStr,
							   const char *uBrainScpWanDesc,const char *uBrainAcpWanDesc)
: EventHandlerT<SessionFactory>(),ThreadSingle()
{
	char	SCPSlaveName[100];
	char	ACPSlaveName[100];
	char	LSCPMasterName[100];

	id = sfIdParam;
	stop = false;
	this->domId = domId;
	strcpy(name,nameParam);
	ownerId = ownerIdParam;

    //If debug dds libs used, set verbosity high
//    NDDSConfigLogger::get_instance()->
//        set_verbosity_by_category(NDDS_CONFIG_LOG_CATEGORY_API,
//                                  NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);

	// Create Session Control Object (SCO) slave
	GEN_CP_INTERFACE_NAME(SCPSlaveName,name,SCP_SLAVE_NAME);
	pSCSlave = new SCPSlave(this,id,domId,SCPSlaveName,"SCP");
    pSCSlave->AddProperty("dds.transport.wan_plugin.wan.transport_instance_id",scpSlaveWanIdStr);
    pSCSlave->AddPeer(uBrainScpWanDesc);
    pSCSlave->Enable();


	// Create Local Session Control Plane (LSCP) master
	GEN_CP_INTERFACE_NAME(LSCPMasterName,name,LSCP_MASTER_NAME);
	pLSCMaster = new LSCPMaster(this,id,domId,LSCPMasterName,"LSCP");
    pLSCMaster->Enable();

	// Create Admin Control Plane (ACP) slave
	GEN_CP_INTERFACE_NAME(ACPSlaveName,name,ACP_SLAVE_NAME);
	pACSlave = new ACPSlave(this,id,domId,ACPSlaveName,"ACP");
    pACSlave->AddProperty("dds.transport.wan_plugin.wan.transport_instance_id",acpSlaveWanIdStr);
    pACSlave->AddPeer(uBrainAcpWanDesc);
    pACSlave->Enable();
	pACSlaveObj = pACSlave->CreateSlaveObject(id);

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

	SetStateStandby();
	//EventHandleLoop();
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
					stringstream    sstream;
					string          Token;

					sstream << pNewSessEvt->GetData()->script;
					sstream >> Token;
					if(Token!="sessname")
					{
					    Token = "SF(";
					    Token = Token + ToString<int>(id) + ")=>SL(";
					    Token = Token + ToString<int>(pSCSlaveObjLocal->GetSessionId()) + ")";
					    cout << "Session name unspecified, using default: " << Token << endl;
					}
					else
					{
					    sstream >> Token;
                        cout << "Given session name: " << Token << endl;
					}

					pSession = new RemoteSessionSF(pSCSlave,pSCSlaveObjLocal,pLSCMaster,domId,id,
												   Token.c_str(),NEW_SL_THREAD);
					SessionList[pSession->GetId()] = pSession;
					std::cout << SF_LOG_PROMPT(id) << ": New Session(" << pSession->GetId() <<
						")" << endl;
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
