#include <queue>
#include <map>
#include <pthread.h>

#define EVENT_KIND_NEW_SESSION		0
#define EVENT_KIND_UPDATE_SESSION	1
#define EVENT_KIND_DELETE_SESSION	2
#define EVENT_KIND_SESSION_INIT		3
#define EVENT_KIND_SESSION_READY	4
#define EVENT_KIND_SESSION_DELETE	5
#define EVENT_KIND_SESSION_DELETED	6
#define EVENT_KIND_FACTORY_SHUTDOWN	7

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

class EventHandlerBase
{
	protected:
		std::queue<Event *>	EventQueue;
		
	public:
		EventHandler()	{ };
		~EventHandler()	{ };
		
virtual	void	SignalEvent	(Event *) = 0;
};

template<typename NeuronEntityType> class EventHandler : public EventHandlerBase
{
	typedef	void (NeuronEntityType::*EventHandleFunc)(Event *);
	
	protected:
		std::queue<Event *>				EventQueue;
		std::map<int,EventHandleFunc>	EventHandleFuncList;
		pthread_mutex_t					eqMutex;
		
	public:
		EventHandler()	{ pthread_mutex_init(&eqMutex, NULL); }
		~EventHandler()	{ }
		
		void	AddHandleFunc	(EventHandleFunc,int);
		void	SignalEvent		(Event *);
		void	HandleNextEvent	(void);
		bool	NoEvents		(void)						{ return EventQueue.empty(); }
virtual void	EventHandleLoop (void) = 0;
};

