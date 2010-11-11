#ifndef SESSIONLEADER_H_
#define SESSIONLEADER_H_

#include "neuroncommon.h"

typedef	long long IDType;

class SessionLeader : public EventHandlerT<SessionLeader>, public ThreadSingle
{
	private:
	
		IDType	id;
		// Local Session Ctrl Object (Slave)
		// List of Entities
		
		void	EventHandleLoop (void);
		int		workerBee		(void)	{ EventHandleLoop(); return 0; }

		/******** Event Handle Functions ************/
		
		// Session Control Plane Slave Event Handlers
		void	HandleNewSessionEvent		(Event *);
		void	HandleUpdateSessionEvent	(Event *);
		void	HandleDeleteSessionEvent	(Event *);	

	public:
	
		SessionLeader(IDType);
		~SessionLeader()		{ std::cout << "Deleted SL for session " << id << endl; }
		
		IDType	GetId(void)		{ return id; }
};

#endif /* SESSIONLEADER_H_ */

