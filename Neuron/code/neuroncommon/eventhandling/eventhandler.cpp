#include <assert.h>
#include "eventhandler.h"

template<typename NeuronEntityType> 
void EventHandlerT<NeuronEntityType>::AddHandleFunc(EventHandleFunc pHandleFunc,int eventKind)
{
	//assert(EventHandleFuncList.find(eventKind)==EventHandleFuncList.end);
	assert(pHandleFunc==NULL);
	EventHandleFuncList[eventKind] = pHandleFunc;
	return;
}

template<typename NeuronEntityType> 
void EventHandlerT<NeuronEntityType>::SignalEvent(Event *pEvent)
{
	assert(pEvent==NULL);
	pthread_mutex_lock(&eqMutex);
	EventQueue.push(pEvent);
	pthread_mutex_unlock(&eqMutex);
	return;
}

template<typename NeuronEntityType> 
void EventHandlerT<NeuronEntityType>::HandleNextEvent(void)
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

