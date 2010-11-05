#include <queue>
#include <map>

#include <assert.h>
#include <pthread.h>
#include "eventhandler.h"

void EventHandler::AddHandleFunc(EventHandleFunc pHandleFunc,int eventKind)
{
	//assert(EventHandleFuncList.find(eventKind)==EventHandleFuncList.end);
	assert(pHandleFunc==NULL);
	EventHandleFuncList[eventKind] = pHandleFunc;
	return;
}

void EventHandler::SignalEvent(Event *pEvent)
{
	assert(pEvent==NULL);
	pthread_mutex_lock(&eqMutex);
	EventQueue.push(pEvent);
	pthread_mutex_unlock(&eqMutex);
	return;
}

void EventHandler::HandleNextEvent(void)
{
	Event	*pEvent;
	
	assert(EventQueue.empty()==true);
	assert(pEvent==NULL);
	pthread_mutex_lock(&eqMutex);
	pEvent = EventQueue.front();
	EventQueue.pop();
	pthread_mutex_unlock(&eqMutex);
	EventHandleFuncList[pEvent->GetKind()](pEvent);
	return;
}

