#include <iostream>
#include <new>
#include <unistd.h>
#include "sessionfactory.h"

SessionFactory::SessionFactory(IDType sfIdParam) : EventHandlerT<SessionFactory>()
{
	id = sfIdParam;
	
	// Create Session Control Object (SCO) slave
	try
	{
		pSCSlave = new SCPSlave(this,id,DOMAIN_ID_SCP,"SCP");
	}
	catch(bad_alloc &baObj)
	{
		std::cerr << "SessionFactory::SessionFactory()/new(SCPSlave): " << baObj.what() << endl;
	}
	// Create Admin Control Object (ACO) slave
	
	// Create Local Session Control Plane (LSCP) master
	
	// Generate hash table of Event Handle Function Ptrs with Event kind as key
	AddHandleFunc(&SessionFactory::HandleNewSessionEvent,SCP_EVENT_NEW_SESSION);
	AddHandleFunc(&SessionFactory::HandleUpdateSessionEvent,SCP_EVENT_UPDATE_SESSION);
	AddHandleFunc(&SessionFactory::HandleDeleteSessionEvent,SCP_EVENT_DELETE_SESSION);
	/*AddHandleFunc(&SessionFactory::HandleSessionInitStateEvent,EVENT_KIND_SESSION_INIT);
	AddHandleFunc(&SessionFactory::HandleSessionReadyStateEvent,EVENT_KIND_SESSION_READY);
	AddHandleFunc(&SessionFactory::HandleSessionDeleteStateEvent,EVENT_KIND_SESSION_DELETE);
	AddHandleFunc(&SessionFactory::HandleSessionDeletedStateEvent,EVENT_KIND_SESSION_DELETED);
	AddHandleFunc(&SessionFactory::HandleFactoryShutdownEvent,EVENT_KIND_FACTORY_SHUTDOWN);*/
	// Create Resource Monitor Object
	//TRY_NEW(pResMtrObj,ResourceMonitor,"SessionFactory::SessionFactory()/new(ResourceMonitor): ");
	
	// Announce yourself on Admin Control Plane
	EventHandleLoop();
}

void SessionFactory::HandleNewSessionEvent(Event *pEvent)
{
	SessionSF			*pSession = NULL;
	SCPEventNewSession 	*pNewSessEvt = NULL;
	
	if(pEvent!=NULL)
	{
		pNewSessEvt = reinterpret_cast<SCPEventNewSession *>(pEvent);

		try
		{
			pSession = new SessionSF(pSCSlave,(SCPSlaveObject *)(pNewSessEvt->GetSession()));
		}
		catch(bad_alloc &baObj)
		{
			std::cerr << "SessionFactory::HAndleNewSessionEvent()/new(SessionSF): " << 
				baObj.what() << endl;
		}
	
		if(SessionList.find(pSession->GetId())!=SessionList.end())
		{
			//TODO: Error, session already present
			delete pSession;
		}
		else
		{
			SessionList[pSession->GetId()] = pSession;
			// 3. Create new session leader
		
			// 4. Send ready state to brain
			pSession->SetStateReady();
			
			std::cout << "New Session with ID " << pSession->GetId() << " created." << endl; 
		}
	}
	else
		std::cout << "New Session: Can't handle null event" << endl;
	
	return;
}

void SessionFactory::HandleUpdateSessionEvent(Event *pEvent)
{
	SessionSF				*pSession = NULL;
	SCPEventUpdateSession	*pUpSessEvt = NULL;
	
	// 1. Get the session object for the requested session id
	if(pEvent!=NULL)
	{
		pUpSessEvt = reinterpret_cast<SCPEventUpdateSession *>(pEvent);
	
		if(SessionList.find(pUpSessEvt->GetData()->sessionId)==SessionList.end())
		{
			//TODO: Error, session not found
		}
		else
		{
			pSession = SessionList[pUpSessEvt->GetData()->sessionId];
			pSession->SetStateUpdate();
			pSession->Update(pUpSessEvt->GetData());
			// 4. Send it to the appropriate session leader
			pSession->SetStateReady();
			std::cout << "Update to Session " << pSession->GetId() << ": " << 
				pUpSessEvt->GetData()->script << endl;
		}
	}
	else
		std::cout << "Update Session: Can't handle null event" << endl;
	
	return;
}

void SessionFactory::HandleDeleteSessionEvent(Event *pEvent)
{
	SessionSF				*pSession = NULL;
	SCPEventDeleteSession	*pDelSessEvt = NULL;
	
	if(pEvent!=NULL)
	{
		pDelSessEvt = reinterpret_cast<SCPEventDeleteSession *>(pEvent);
		if(SessionList.find(pDelSessEvt->GetSessionId())==SessionList.end())
		{
			//TODO: Error, session not found
		}
		else
		{
			pSession = SessionList[pDelSessEvt->GetSessionId()];
			SessionList.erase(pDelSessEvt->GetSessionId());
			delete pSession;
			std::cout << "Session " << pDelSessEvt->GetSessionId() << " deleted." << endl;	
		}
	}
	else
		std::cout << "Delete Session: Can't handle null event" << endl;
		
	return;	
}

/*void SessionFactory::HandleSessionInitStateEvent(Event *pEvent)
{

}

void SessionFactory::HandleSessionReadyStateEvent(Event *pEvent)
{

}

void SessionFactory::HandleSessionUpdateStateEvent(Event *pEvent)
{

}

void SessionFactory::HandleSessionDeleteStateEvent(Event *pEvent)
{

}

void SessionFactory::HandleSessionDeletedStateEvent(Event *pEvent)
{

}

void SessionFactory::HandleFactoryShutdownEvent(Event *pEvent)
{

}*/

void SessionFactory::EventHandleLoop(void)
{
	while(1)
	{
		//wait while event queue is empty
		while(NoEvents())
		{
			usleep(EVENTQ_SLEEP_MUS);
		}
		
		HandleNextEvent();
	}
}

