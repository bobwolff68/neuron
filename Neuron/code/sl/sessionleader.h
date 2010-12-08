#ifndef SESSIONLEADER_H_
#define SESSIONLEADER_H_

#include <string.h>
#include "ndds_cpp.h"
#include "neuroncommon.h"
#include "controlplane.h"
#include "natnumsrc.h"
#include "stdoutsink.h"

#define DOMAIN_ID_LSCP	11

#define SL_LOG_PROMPT(oId,id)	"SF(" << oId << ")->SL(" << id << ")"

typedef	long long IDType;

class SessionLeader : public EventHandlerT<SessionLeader>, public ThreadSingle
{
	private:
	
		IDType				id;
		IDType				sessionId;
		IDType				ownerId;
		char		   	   	name[100];
		LSCPSlave	   	   *pLSCSlave;
		LSCPSlaveObject	   *pLSCSlaveObj;
		
		DDSDomainParticipant		   *pMediaDP;
		std::map<int,SessionEntity*> 	EntityList;
		std::map<int,DDSTopic*>			TopicList;
		
		com::xvd::neuron::lscp::Control *control;
    	com::xvd::neuron::lscp::State   *state;
    	com::xvd::neuron::lscp::Event   *event;
    	com::xvd::neuron::lscp::Metrics *metrics;		
		
		void	EventHandleLoop (void);
		int		workerBee		(void)	{ EventHandleLoop(); return 0; }

		/******** Event Handle Functions ************/
		
		// Session Control Plane Slave Event Handlers
		void	HandleNewSessionEvent		(Event *);
		void	HandleUpdateSessionEvent	(Event *);
		void	HandleDeleteSessionEvent	(Event *);
		void	ProcessScript				(const char *);

	public:
	
		SessionLeader(IDType,IDType,const char *,int,int);
		~SessionLeader();
		
		IDType	GetId(void)			{ return id; }

		IDType	GetSessionId(void)	{ return sessionId; }
		
    	void Update(com::xvd::neuron::lscp::Control *control)
    	{
		    strcpy(this->control->script,control->script);
		    std::cout << SL_LOG_PROMPT(ownerId,sessionId) << "(update): " << control->script << endl;
		     
		    return;
    	}
    	
    	void SetStateStandby(void)
    	{
    		state->state = com::xvd::neuron::OBJECT_STATE_STANDBY;
		    pLSCSlaveObj->Send(state);
		    std::cout << SL_LOG_PROMPT(ownerId,sessionId) << ": STDBY (LSCP)" << endl; 
		    
		    return;
    	}
    	
    	void SetStateReady(void)
    	{
    		state->state = com::xvd::neuron::OBJECT_STATE_READY;
		    pLSCSlaveObj->Send(state);
		    std::cout << SL_LOG_PROMPT(ownerId,sessionId) << ": READY (LSCP)" << endl; 
		    
		    return;
    	}
    	
    	void SetStateUpdate(void)
    	{
    		state->state = com::xvd::neuron::OBJECT_STATE_UPDATE;
		    pLSCSlaveObj->Send(state);
		    std::cout << SL_LOG_PROMPT(ownerId,sessionId) << ": UPDATE (LSCP)" << endl; 
		    
		    return;
    	}
    	
    	void SetStateDelete(void)
    	{
    		state->state = com::xvd::neuron::OBJECT_STATE_DELETE;
		    pLSCSlaveObj->Send(state);
		    std::cout << SL_LOG_PROMPT(ownerId,sessionId) << ": DELETE (LSCP)" << endl; 
		    
		    return;
    	}

    	void SetStateDeleted(void)
    	{
    		state->state = com::xvd::neuron::OBJECT_STATE_DELETED;
		    pLSCSlaveObj->Send(state);
		    std::cout << SL_LOG_PROMPT(ownerId,sessionId) << ": DELETED (LSCP)" << endl; 
		    
		    return;
    	}
};

#endif /* SESSIONLEADER_H_ */

