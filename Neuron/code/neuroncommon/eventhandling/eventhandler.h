//!
//! \file 	eventhandler.h
//!
//! \brief	Definition of the Event Handler Template
//!
//! \author Manjesh Malavalli (mmalavalli@xvdth.com)
//! \date  	11/01/2010
//!

#ifndef EVENTHANDLER_H_
#define EVENTHANDLER_H_

#include <iostream>
#include <queue>
#include <map>
#include <assert.h>
#include <pthread.h>

#define	EVENTQ_SLEEP_MUS	100000

class Event
{
	protected:
	
		int	kind;
		
	public:
	
		Event(int kindParam)	{ kind = kindParam; }
		~Event()				{ }
		
		int	GetKind	(void)		{ return kind; }
};

// Base class solely for passing reference to EventHandler
// SF/SL inherits EventHandlerT, so since a ptr to template can't be passed,
// this base class is created to overcome that problem.
class EventHandler
{
	protected:
	
		std::queue<Event *>	EventQueue;
		
	public:
	
		EventHandler()	{ };
		~EventHandler()	{ while(!EventQueue.empty())	EventQueue.pop(); };
		
virtual	void	SignalEvent	(Event *) = 0;
};

template<typename NeuronEntityType> class EventHandlerT : public EventHandler
{
	public: typedef	void (NeuronEntityType::*EventHandleFunc)(Event *);
	
	protected:
	
		std::queue<Event *>				EventQueue;
		std::map<int,EventHandleFunc>	EventHandleFuncList;
		pthread_mutex_t					eqMutex;
		
	public:

		EventHandlerT()		{ pthread_mutex_init(&eqMutex, NULL); }
		~EventHandlerT()	{ }

		void	AddHandleFunc	(EventHandleFunc,int);
		void	SignalEvent		(Event *);
		void	HandleNextEvent	(void);
		bool	NoEvents		(void)						{ return EventQueue.empty(); }
virtual void	EventHandleLoop (void) = 0;
};

template<typename NeuronEntityType>
void EventHandlerT<NeuronEntityType>::AddHandleFunc(EventHandleFunc pHandleFunc,int eventKind)
{
	if(pHandleFunc!=NULL)
		EventHandleFuncList[eventKind] = pHandleFunc;
	else
		std::cout << "Can't add null event handle function" << std::endl;
		
	return;
}

template<typename NeuronEntityType>
void EventHandlerT<NeuronEntityType>::SignalEvent(Event *pEvent)
{
	if(pEvent!=NULL)
	{
		pthread_mutex_lock(&eqMutex);
		EventQueue.push(pEvent);
		pthread_mutex_unlock(&eqMutex);
	}
	else
		std::cout << "Can't enqueue null event" << std::endl;
		
	return;
}

template<typename NeuronEntityType>
void EventHandlerT<NeuronEntityType>::HandleNextEvent(void)
{
    Event   *pEvent = NULL;

	if (EventQueue.empty())
		return;

        pthread_mutex_lock(&eqMutex);
        pEvent = EventQueue.front();
	if (pEvent != NULL) {
       	    EventQueue.pop();
            pthread_mutex_unlock(&eqMutex);
	    if (EventHandleFuncList[pEvent->GetKind()])
		{
	    		(((NeuronEntityType*)this)->*EventHandleFuncList[pEvent->GetKind()])(pEvent);
		}
	} 
	else
	{
    	pthread_mutex_lock(&eqMutex);
    	pEvent = EventQueue.front();
   	    EventQueue.pop();
   	    pthread_mutex_unlock(&eqMutex);
        
		if (pEvent != NULL) 
		{
    	    if(EventHandleFuncList.find(pEvent->GetKind())!=EventHandleFuncList.end())
			    (((NeuronEntityType*)this)->*EventHandleFuncList[pEvent->GetKind()])(pEvent);
			else
				std::cout << "Handle undefined for event type " << pEvent->GetKind() << std::endl;
		}
		else
			std::cout << "Can't handle null event" << std::endl;
	}
	
	delete pEvent;
	return;
}

#endif /* EVENTHANDLER_H_ */

