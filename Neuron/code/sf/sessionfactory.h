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
#include "neuroncommon.h"
#include "controlplane.h"
#include "sessionleader.h"

#define DOMAIN_ID_SCP 	10
#define DOMAIN_ID_ACP	12

#define NEW_SL_THREAD 	1
#define NEW_SL_PROCESS	2

#define SF_LOG_PROMPT(id)	"SF(" << id << ")"
#define SO_LOG_PROMPT(id)	"SO(" << id << ")"

typedef	long long IDType;


/*class ResourceMonitor
{

};*/

class SessionFactory : public EventHandlerT<SessionFactory>
{
	class RemoteSessionSF
	{
		class SessionLeaderRef
		{
			public:
	
				SessionLeader  *pSL;
				int				createMode;
				//Process ID
				//Thread Object
		
				SessionLeaderRef()		{ pSL = NULL; }
				~SessionLeaderRef()		{ }
		};

		private:
		
			SCPSlave		   *pSCSlave;
			SCPSlaveObject 	   *pSCSlaveObj;
			LSCPMaster		   *pLSCMaster;
			LSCPMasterObject   *pLSCMasterObj;
			SessionLeaderRef	slRef;
		
			com::xvd::neuron::scp::Control *control;
			com::xvd::neuron::scp::State   *state;
			com::xvd::neuron::scp::Event   *event;
			com::xvd::neuron::scp::Metrics *metrics;

			com::xvd::neuron::lscp::Control *lscControl;
			com::xvd::neuron::lscp::State   *lscState;
			com::xvd::neuron::lscp::Event   *lscEvent;
			com::xvd::neuron::lscp::Metrics *lscMetrics;
			
		public:
			
			RemoteSessionSF(SCPSlave *pSCSlaveParam,SCPSlaveObject *pSCSlaveObjParam,
					  		LSCPMaster *pLSCMasterParam,int domId,int slCreateMode)
			{
				pSCSlave = pSCSlaveParam;
				pSCSlaveObj = pSCSlaveObjParam;
				pLSCMaster = pLSCMasterParam;
						    
				// Initialize SCP instances
				control = com::xvd::neuron::scp::ControlTypeSupport::create_data();
				state = com::xvd::neuron::scp::StateTypeSupport::create_data();
				event = com::xvd::neuron::scp::EventTypeSupport::create_data();
				metrics = com::xvd::neuron::scp::MetricsTypeSupport::create_data();

				// Initialize LSCP instances
				lscControl = com::xvd::neuron::lscp::ControlTypeSupport::create_data();
				lscState = com::xvd::neuron::lscp::StateTypeSupport::create_data();
				lscEvent = com::xvd::neuron::lscp::EventTypeSupport::create_data();
				lscMetrics = com::xvd::neuron::lscp::MetricsTypeSupport::create_data();
				
				slRef.createMode = slCreateMode;
				
				// Create the LSCP Master Object for the Session
				pLSCMasterObj = pLSCMaster->CreateMasterObject(GetId());
			
				// Create a new SL
				if(slCreateMode==NEW_SL_THREAD)
				{
					slRef.pSL = new SessionLeader(GetId(),GetId(),domId);
					(slRef.pSL)->startThread();
				}
				else
				{
					pid_t	pId;
					char	lscpSessIdStr[50];
					
					slRef.pSL = NULL;
					sprintf(lscpSessIdStr,"%d",GetId());

					if((pId=fork())==0)
					{
						execlp("../bin/sl","../bin/sl",lscpSessIdStr,(char *)NULL);
						exit(0);
					}
					else if(pId<0)
						std::cout << SO_PROMPT(GetId) << ": Fork fail" << endl;
				}
				
				// Set state to INIT until SL is successfully created
				SetStateInit();
			}
			
			~RemoteSessionSF()
			{
				if(slRef.createMode==NEW_SL_THREAD)
				{
					(slRef.pSL)->stopThread();
					delete slRef.pSL;
				}
				else
				{
					;// Send termination message
				}
			
				pSCSlave->DeleteSlaveObject(pSCSlaveObj);
				pLSCMaster->DeleteMasterObject(pLSCMasterObj);

				// Destroy SCP instances			
				com::xvd::neuron::scp::ControlTypeSupport::delete_data(control);
				com::xvd::neuron::scp::StateTypeSupport::delete_data(state);
				com::xvd::neuron::scp::EventTypeSupport::delete_data(event);
				com::xvd::neuron::scp::MetricsTypeSupport::delete_data(metrics);
				
	   			// Destroy LSCP instances			
				com::xvd::neuron::lscp::ControlTypeSupport::delete_data(lscControl);
				com::xvd::neuron::lscp::StateTypeSupport::delete_data(lscState);
				com::xvd::neuron::lscp::EventTypeSupport::delete_data(lscEvent);
				com::xvd::neuron::lscp::MetricsTypeSupport::delete_data(lscMetrics);     
			}
			
			void Update(com::xvd::neuron::scp::Control *control)
			{
				strcpy(this->control->script,control->script);
				strcpy(lscControl->script,control->script);
				pLSCMasterObj->Send(lscControl,GetId());
				std::cout << SO_LOG_PROMPT(GetId()) << ": " << control->script << endl;
				return;
			}
			
			void SetStateInit(void)
			{
				state->state = com::xvd::neuron::OBJECT_STATE_INIT;
				pSCSlaveObj->Send(state);
				std::cout << SO_LOG_PROMPT(GetId()) << ": INIT (SCP)" << endl; 
				
				return;
			}
			
			void SetStateReady(void)
			{
				state->state = com::xvd::neuron::OBJECT_STATE_READY;
				pSCSlaveObj->Send(state);
				std::cout << SO_LOG_PROMPT(GetId()) << ": READY (SCP)" << endl; 
				
				return;
			}
			
			void SetStateUpdate(void)
			{
				state->state = com::xvd::neuron::OBJECT_STATE_UPDATE;
				pSCSlaveObj->Send(state);
				std::cout << SO_LOG_PROMPT(GetId()) << ": UPDATE (SCP)" << endl; 
				
				return;
			}
			
			void HandleSLState(com::xvd::neuron::lscp::State *slState)
			{
				lscState->state = slState->state;
				
				switch(lscState->state)
				{
					case com::xvd::neuron::OBJECT_STATE_READY:
						SetStateReady();
						break;
						
					case com::xvd::neuron::OBJECT_STATE_STANDBY:
						strcpy(lscControl->script,"PING");
						pLSCMasterObj->Send(lscControl,GetId());
						std::cout << SO_LOG_PROMPT(GetId()) << ": " << lscControl->script << endl;
						break;
				}
				
				return;
			}
			
			int GetId(void)	
			{ 
				return pSCSlaveObj->GetSessionId(); 
			}
	};

	private:
	
		IDType						id;
		bool						stop;
		int							domId; 
		SCPSlave   				   *pSCSlave;
		LSCPMaster			   	   *pLSCMaster;
		ACPSlave				   *pACSlave;
		ACPSlaveObject			   *pACSlaveObj;
		map<int,RemoteSessionSF *>	SessionList;
		//ResourceMonitor	   *pResMtrObj;
		
		com::xvd::neuron::acp::Control *acControl;
		com::xvd::neuron::acp::State   *acState;
    	com::xvd::neuron::acp::Event   *acEvent;
    	com::xvd::neuron::acp::Metrics *acMetrics;
		
		void	EventHandleLoop (void);
		
		/******** Event Handle Functions ************/
		
		// Session Control Plane Slave Event Handlers
		void	HandleNewSessionEvent		(Event *);
		void	HandleUpdateSessionEvent	(Event *);
		void	HandleDeleteSessionEvent	(Event *);
		
		// Local Session Control Plane Master Event Handlers
		void	HandleSessionUpdateStateEvent	(Event *);
		
		// Admin Control Plane Slave Event Handlers		
		void	HandleACPNewSessionEvent	(Event *);
		void	HandleACPUpdateSessionEvent	(Event *);
		void	HandleACPDeleteSessionEvent	(Event *);
				
	public:

		SessionFactory(IDType,int);
		~SessionFactory();
		
		IDType	GetId(void)		{ return id; }
		
		void SetStateStandby(void)
		{
    		acState->state = com::xvd::neuron::OBJECT_STATE_STANDBY;
		    pACSlaveObj->Send(acState);
		    std::cout << SF_LOG_PROMPT(id) << ": STANDBY (ACP)" << endl; 
		    
		    return;		
		}

		void SetStateReady(void)
		{
    		acState->state = com::xvd::neuron::OBJECT_STATE_READY;
		    pACSlaveObj->Send(acState);
		    std::cout << SF_LOG_PROMPT(id) << ": READY (ACP)" << endl; 
		    
		    return;		
		}
		
		void SetStateDeleted(void)
		{
    		acState->state = com::xvd::neuron::OBJECT_STATE_DELETED;
		    pACSlaveObj->Send(acState);
		    std::cout << SF_LOG_PROMPT(id) << ": DELETED (ACP)" << endl; 
		    
		    return;		
		}
};

#endif /* SESSIONFACTORY_H_ */

