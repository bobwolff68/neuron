#include <iostream>
#include <new>
#include <unistd.h>
#include "sessionleader.h"

SessionLeader::SessionLeader(IDType slIdParam) : EventHandlerT<SessionLeader>(),ThreadSingle()
{
	id = slIdParam;
	
	std::cout << "Created SL for session " << id << endl;
	// Create Local Session Control Plane (LSCP) slave
	
	// Generate hash table of Event Handle Function Ptrs with Event kind as key
	//AddHandleFunc(&SessionLeader::HandleNewSessionEvent,EVENT_KIND_NEW_SESSION);
	//AddHandleFunc(&SessionLeader::HandleUpdateSessionEvent,EVENT_KIND_UPDATE_SESSION);
	//AddHandleFunc(&SessionLeader::HandleDeleteSessionEvent,EVENT_KIND_DELETE_SESSION);
	
	// Send setup success to session factory
}

void SessionLeader::HandleNewSessionEvent(Event *pEvent)
{
	std::cout << "Created SessionSL object for session " << id << endl;
	return;
}

void SessionLeader::HandleUpdateSessionEvent(Event *pEvent)
{
	std::cout << "Updated SessionSL object for session " << id << endl;
	return;
}

void SessionLeader::HandleDeleteSessionEvent(Event *pEvent)
{
	std::cout << "Deleted SessionSL object for session " << id << endl;
	return;
}

void SessionLeader::EventHandleLoop(void)
{
	while(!isStopRequested)
	{
		//wait while event queue is empty
		while(NoEvents()&&!isStopRequested)
		{
			usleep(EVENTQ_SLEEP_MUS);
		}

		if(!isStopRequested)		
			HandleNextEvent();
	}
	
	return;
}

