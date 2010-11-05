#define EVENT_KIND_NEW_SESSION
#define EVENT_KIND_UPDATE_SESSION
#define EVENT_KIND_DELETE_SESSION
#define EVENT_KIND_SESSION_INIT
#define EVENT_KIND_SESSION_READY
#define EVENT_KIND_SESSION_DELETE
#define EVENT_KIND_SESSION_DELETED
#define EVENT_KIND_FACTORY_SHUTDOWN

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

typedef	void (*EventHandleFunc)(Event *) ;

class EventHandler
{
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

