#include <iostream>
#include <new>
#include <unistd.h>
#include "sessionfactory.h"

SessionFactory::SessionFactory(IDType sfIdParam,int domId) : EventHandlerT<SessionFactory>()
{
	id = sfIdParam;
	stop = false;
	this->domId = domId;
	
	// Create Session Control Object (SCO) slave
	pSCSlave = new SCPSlave(this,id,domId,"SCP");
	
	// Create Local Session Control Plane (LSCP) master
	pLSCMaster = new LSCPMaster(this,id,domId,"LSCP");
	
	// Create Admin Control Plane (ACP) slave
	pACSlave = new ACPSlave(this,id,domId,"ACP");	
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
	EventHandleLoop();
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
	
	delete pLSCMaster;
	delete pSCSlave;
	SetStateDeleted();
	pACSlave->DeleteSlaveObject(pACSlaveObj);
	delete pACSlave;
}

void SessionFactory::HandleNewSessionEvent(Event *pEvent)
{
	RemoteSessionSF			*pSession = NULL;
	SCPEventNewSession 	*pNewSessEvt = NULL;
	
	if(pEvent!=NULL)
	{
		pNewSessEvt = reinterpret_cast<SCPEventNewSession *>(pEvent);

		// Creating a new RemoteSessionSF Object will create an SL for that session
		pSession = new RemoteSessionSF(pSCSlave,(SCPSlaveObject *)(pNewSessEvt->GetSession()),
									   pLSCMaster,domId,NEW_SL_THREAD);
	
		if(SessionList.find(pSession->GetId())!=SessionList.end())
		{
			// Error, session already present
			std::cout << SF_LOG_PROMPT(id) << "(NEWSESSION): Session " << pSession->GetId() << 
				" already present" << endl;
			delete pSession;
		}
		else
		{
			SessionList[pSession->GetId()] = pSession;
			std::cout << SF_LOG_PROMPT(id) << ": New Session(" << pSession->GetId() << ")" << endl; 
		}
	}
	else
	{
		std::cout << "New Session: Can't handle null event" << endl;
	}
	
	return;
}

void SessionFactory::HandleUpdateSessionEvent(Event *pEvent)
{
	RemoteSessionSF				*pSession = NULL;
	SCPEventUpdateSession	*pUpSessEvt = NULL;
	
	//Get the session object for the requested session id
	if(pEvent!=NULL)
	{
		pUpSessEvt = reinterpret_cast<SCPEventUpdateSession *>(pEvent);
	
		if(SessionList.find(pUpSessEvt->GetData()->sessionId)==SessionList.end())
		{
			//TError, session not found
			std::cout << SF_LOG_PROMPT(id) << "(UPSESSION): Session " << 
				pUpSessEvt->GetData()->sessionId << " not found" << endl;
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
	else
		std::cout << "Update Session: Can't handle null event" << endl;
	
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
		std::cout << "Delete Session: Can't handle null event" << endl;
		
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
		std::cout << "Update State: Can't handle null event" << endl;
		
	return;	
}

void SessionFactory::HandleACPNewSessionEvent(Event *pEvent)
{
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
	stop = true;
	return;
}

void SessionFactory::EventHandleLoop(void)
{
	while(!stop)
	{
		//wait while event queue is empty
		while(NoEvents()&&!stop)
		{
			usleep(EVENTQ_SLEEP_MUS);
		}
		
		if(!stop)
			HandleNextEvent();
	}
	
	return;
}

