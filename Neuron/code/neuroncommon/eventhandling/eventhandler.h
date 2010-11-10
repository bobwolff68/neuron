#ifndef EVENTHANDLER_H_
#define EVENTHANDLER_H_

#include <queue>
#include <map>

#include <assert.h>
#include <pthread.h>
#include "eventhandler.h"

/*#define EVENT_KIND_NEW_SESSION		0
#define EVENT_KIND_UPDATE_SESSION	1
#define EVENT_KIND_DELETE_SESSION	2
#define EVENT_KIND_SESSION_INIT		3
#define EVENT_KIND_SESSION_READY	4
#define EVENT_KIND_SESSION_DELETE	5
#define EVENT_KIND_SESSION_DELETED	6
#define EVENT_KIND_FACTORY_SHUTDOWN	7*/

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
		~EventHandler()	{ };
		
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
        //assert(EventHandleFuncList.find(eventKind)==EventHandleFuncList.end);
        assert(pHandleFunc!=NULL);
        EventHandleFuncList[eventKind] = pHandleFunc;
        return;
}

template<typename NeuronEntityType>
void EventHandlerT<NeuronEntityType>::SignalEvent(Event *pEvent)
{
        assert(pEvent!=NULL);
        pthread_mutex_lock(&eqMutex);
        EventQueue.push(pEvent);
        pthread_mutex_unlock(&eqMutex);
        return;
}

template<typename NeuronEntityType>
void EventHandlerT<NeuronEntityType>::HandleNextEvent(void)
{
    Event   *pEvent=NULL;

	if (EventQueue.empty())
		return;

    pthread_mutex_lock(&eqMutex);
    pEvent = EventQueue.front();
        
	if (pEvent != NULL) 
	{
        EventQueue.pop();
        pthread_mutex_unlock(&eqMutex);
        if(EventHandleFuncList.find(pEvent->GetKind())!=EventHandleFuncList.end())
		    (((NeuronEntityType*)this)->*EventHandleFuncList[pEvent->GetKind()])(pEvent);
		else
			std::cout << "Handle undefined for event type " << pEvent->GetKind() << endl;
	} 
	else
	{
	    pthread_mutex_unlock(&eqMutex);
	}

        return;
}

#endif /* EVENTHANDLER_H_ */

