//!
//! \file 	sessionfactory.h
//!
//! \brief	Definition of the Session Factory
//!
//! \author Manjesh Malavalli (mmalavalli@xvdth.com)
//! \date  	11/01/2010
//!

#ifndef SESSIONFACTORY_H_
#define SESSIONFACTORY_H_

#include <string.h>
#include "ndds_cpp.h"
#include "SCPInterface.h"
#include "SCPSlave.h"
#include "SCPEvent.h"
#include "neuroncommon.h"
//#include "sessionleader.h"

#define DOMAIN_ID_SCP 10

typedef	long long IDType;

class SessionSF
{
	private:
		
		SCPSlave   	   *pSCSlave;
		SCPSlaveObject *pSCSlaveObj;
		
		com::xvd::neuron::session::Control *control;
    	com::xvd::neuron::session::State   *state;
    	com::xvd::neuron::session::Event   *event;
    	com::xvd::neuron::session::Metrics *metrics;
    	
    public:
    	
    	SessionSF(SCPSlave *pSCSlaveParam,SCPSlaveObject *pSCSlaveObjParam)
    	{
		    pSCSlave = pSCSlaveParam;
		    pSCSlaveObj = pSCSlaveObjParam;
		    
		    control = com::xvd::neuron::session::ControlTypeSupport::create_data();
		    state = com::xvd::neuron::session::StateTypeSupport::create_data();
		    event = com::xvd::neuron::session::EventTypeSupport::create_data();
		    metrics = com::xvd::neuron::session::MetricsTypeSupport::create_data();
		    
		    sprintf(control->script,"Create session %d",pSCSlaveObj->GetSessionId());    		
    	}
    	
    	~SessionSF()
    	{
			pSCSlave->DeleteSlaveObject(pSCSlaveObj);
		    com::xvd::neuron::session::ControlTypeSupport::delete_data(control);
		    com::xvd::neuron::session::StateTypeSupport::delete_data(state);
		    com::xvd::neuron::session::EventTypeSupport::delete_data(event);
		    com::xvd::neuron::session::MetricsTypeSupport::delete_data(metrics);     
    	}
    	
    	void Update(com::xvd::neuron::session::Control *control)
    	{
		    strcpy(this->control->script,control->script);
    	}
    	
    	void SetStateReady(void)
    	{
    		state->state = com::xvd::neuron::OBJECT_STATE_READY;
		    pSCSlaveObj->Send(state);
    	}
    	
    	void SetStateUpdate(void)
    	{
    		state->state = com::xvd::neuron::OBJECT_STATE_UPDATE;
		    pSCSlaveObj->Send(state);    	
    	}
    	
    	int GetId(void)	
    	{ 
    		return pSCSlaveObj->GetSessionId(); 
    	}
};

class ResourceMonitor
{

};

class SessionFactory : public EventHandlerT<SessionFactory>
{
	private:
	
		IDType					id;
		SCPSlave   			   *pSCSlave;
		map<int,SessionSF *>	SessionList;
		//ResourceMonitor	   *pResMtrObj;
		// Admin Ctrl Object (Slave)
		// Local Session Ctrl Object (Master)
		
		void	EventHandleLoop (void);
		
		/******** Event Handle Functions ************/
		
		// Session Control Plane Slave Event Handlers
		void	HandleNewSessionEvent		(Event *);
		void	HandleUpdateSessionEvent	(Event *);
		void	HandleDeleteSessionEvent	(Event *);
		
		// Local Session Control Plane Master Event Handlers
		//void	HandleSessionInitStateEvent		(Event *);
		//void	HandleSessionReadyStateEvent	(Event *);
		//void	HandleSessionUpdateStateEvent	(Event *);
		//void	HandleSessionDeleteStateEvent	(Event *);
		//void	HandleSessionDeletedStateEvent	(Event *);
		
		// Admin Control Plane Slave Event Handlers		
		//void	HandleFactoryShutdownEvent	(Event *);
				
	public:

		SessionFactory(IDType);
		~SessionFactory()		{ delete pSCSlave; }
		
		IDType	GetId(void)		{ return id; }
};

#endif /* SESSIONFACTORY_H_ */


