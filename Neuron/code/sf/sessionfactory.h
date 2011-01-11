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

#define NEW_SL_THREAD 	1
#define NEW_SL_PROCESS	2

#define SF_LOG_PROMPT(id)		"SF(" << id << ")"
#define SO_LOG_PROMPT(oId,id)	SF_LOG_PROMPT(oId) << "->SO(" << id << ")"

typedef	long long IDType;


/*class ResourceMonitor
{

};*/

class SessionFactory : public EventHandlerT<SessionFactory>, public ThreadSingle
{
	class RemoteSessionSF
	{
		class SessionLeaderRef
		{
			public:

				SessionLeader  *pSL;
				int				createMode;

				SessionLeaderRef()		{ pSL = NULL; }
				~SessionLeaderRef()		{ }
		};

		private:

			IDType				ownerId;
			SCPSlave		   *pSCSlave;
			SCPSlaveObject 	   *pSCSlaveObj;
			LSCPMaster		   *pLSCMaster;
			LSCPMasterObject   *pLSCMasterObj;

			com::xvd::neuron::scp::Control *control;
			com::xvd::neuron::scp::State   *state;
			com::xvd::neuron::scp::Event   *event;
			com::xvd::neuron::scp::Metrics *metrics;

			com::xvd::neuron::lscp::Control *lscControl;
			com::xvd::neuron::lscp::State   *lscState;
			com::xvd::neuron::lscp::Event   *lscEvent;
			com::xvd::neuron::lscp::Metrics *lscMetrics;

		public:

			SessionLeaderRef	slRef;

			RemoteSessionSF(SCPSlave *pSCSlaveParam,SCPSlaveObject *pSCSlaveObjParam,
					  		LSCPMaster *pLSCMasterParam,int domIdParam,int ownerIdParam,
					  		const char *nameParam,int slCreateMode)
			{
				ownerId = ownerIdParam;
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
					/*char slName[100];
					char idStr[20];

					strcpy(slName,"SF(");
					sprintf(idStr,"%lld",ownerId);
					strcat(slName,idStr);
					strcat(slName,")=>SL(");
					sprintf(idStr,"%lld",GetId());
					strcat(slName,idStr);
					strcat(slName,")");*/

					slRef.pSL = new SessionLeader(ownerId,GetId(),nameParam,domIdParam,ownerId);
					(slRef.pSL)->startThread();
				}
				else
				{
					pid_t	pId;
					char	lscpSessIdStr[50];

					slRef.pSL = NULL;
					sprintf(lscpSessIdStr,"%lld",GetId());

					if((pId=fork())==0)
					{
						execlp("../bin/sl","../bin/sl",lscpSessIdStr,(char *)NULL);
						exit(0);
					}
					else if(pId<0)
						std::cout << SO_LOG_PROMPT(ownerId,GetId()) << ": Fork fail" << endl;
				}

				// Set state to INIT until SL is successfully created
				SetStateInit();
			}

			~RemoteSessionSF()
			{
				if(slRef.createMode==NEW_SL_THREAD)
				{
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
				pLSCMasterObj->Send(lscControl,ownerId);
				std::cout << SO_LOG_PROMPT(ownerId,GetId()) << ": " << control->script << endl;
				return;
			}

			void SetStateInit(void)
			{
				state->state = com::xvd::neuron::OBJECT_STATE_INIT;
				pSCSlaveObj->Send(state);
				std::cout << SO_LOG_PROMPT(ownerId,GetId()) << ": INIT (SCP)" << endl;

				return;
			}

			void SetStateReady(void)
			{
				state->state = com::xvd::neuron::OBJECT_STATE_READY;
				pSCSlaveObj->Send(state);
				std::cout << SO_LOG_PROMPT(ownerId,GetId()) << ": READY (SCP)" << endl;

				return;
			}

			void SetStateUpdate(void)
			{
				state->state = com::xvd::neuron::OBJECT_STATE_UPDATE;
				pSCSlaveObj->Send(state);
				std::cout << SO_LOG_PROMPT(ownerId,GetId()) << ": UPDATE (SCP)" << endl;

				return;
			}

			void OfferSource(const char *srcDescriptor)
			{
				state->state = com::xvd::neuron::OBJECT_STATE_OFFERSRC;
				strcpy(state->payload,srcDescriptor);
				pSCSlaveObj->Send(state);
				std::cout << SO_LOG_PROMPT(ownerId,GetId()) << ": OFFERSRC => " << srcDescriptor << " (SCP)" << endl;

				return;
			}

			void SelectRemoteSource(const char *rmtSrcDescriptor)
			{
				state->state = com::xvd::neuron::OBJECT_STATE_SELECTSRC;
				strcpy(state->payload,rmtSrcDescriptor);
				pSCSlaveObj->Send(state);
				std::cout << SO_LOG_PROMPT(ownerId,GetId()) << ": SELECTSRC => " << rmtSrcDescriptor << " (SCP)" << endl;

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
						if(state->state==com::xvd::neuron::OBJECT_STATE_INIT)
						{
							strcpy(lscControl->script,"PING");
							pLSCMasterObj->Send(lscControl,ownerId);
							std::cout << SO_LOG_PROMPT(ownerId,GetId()) << ": " <<
								lscControl->script << " @ STANDBY" << endl;
						}
						else
							std::cout << SO_LOG_PROMPT(ownerId,GetId()) << ": NOT INIT" << endl;
						break;

					case com::xvd::neuron::OBJECT_STATE_OFFERSRC:
						std::cout << "Offer: " << slState->payload << std::endl;
						OfferSource(slState->payload);
						break;

					case com::xvd::neuron::OBJECT_STATE_SELECTSRC:
						std::cout << "Select: " << slState->payload << std::endl;
						SelectRemoteSource(slState->payload);
						break;
				}

				return;
			}

			IDType GetId(void)
			{
				return pSCSlaveObj->GetSessionId();
			}

			IDType GetOwnerId(void)
			{
				return ownerId;
			}
	};

	private:

		IDType						id;
		IDType						ownerId;
		int							domId;
		char					   	name[100];
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

		int workerBee(void)
		{
			EventHandleLoop();
			return 0;
		}

	public:

		bool stop;

		SessionFactory(IDType,const char *,IDType,int,const char *,const char *,
                       const char *,const char *);
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

		void SetStateUpdate(void)
		{
    		acState->state = com::xvd::neuron::OBJECT_STATE_UPDATE;
		    pACSlaveObj->Send(acState);
		    std::cout << SF_LOG_PROMPT(id) << ": UPDATE (ACP)" << endl;

		    return;
		}

		void SetStateDeleted(void)
		{
    		acState->state = com::xvd::neuron::OBJECT_STATE_DELETED;
		    pACSlaveObj->Send(acState);
		    std::cout << SF_LOG_PROMPT(id) << ": DELETED (ACP)" << endl;

		    return;
		}

		int GetEntityIdForRmtSrc(int sessionId,const char *srcName)
		{
			int id = -1;

			if(SessionList.find(sessionId)!=SessionList.end())
				id = SessionList[sessionId]->slRef.pSL->GetEntityIdForRmtSrc(srcName);

			return id;
		}

		bool GetRmtSrcEntIdList(int sessionId,std::map<std::string,int> &List)
		{
			bool found = false;

			if(SessionList.find(sessionId)!=SessionList.end())
			{
				SessionList[sessionId]->slRef.pSL->GetRmtSrcEntIdList(List);
				found = true;
			}

			return found;
		}

		bool SetH264DecoderSinkSubLayerULimit(int sessionId,int epSrcId,int layerULimit)
		{
			bool retVal = false;

			if(SessionList.find(sessionId)!=SessionList.end())
				retVal = SessionList[sessionId]->slRef.pSL->SetH264DecoderSinkSubLayerULimit(epSrcId,layerULimit);

			return retVal;
		}

		int GetTimesParsed(int sessionId,int srcEntId)
		{
			if(SessionList.find(sessionId)!=SessionList.end())
				return SessionList[sessionId]->slRef.pSL->GetTimesParsed(srcEntId);

			return -1;
		}

		void RemoveH264DecoderSink(int sessionId,int epSrcId)
		{
			if(SessionList.find(sessionId)!=SessionList.end())
				SessionList[sessionId]->slRef.pSL->RemoveH264DecoderSink(epSrcId);

			return;
		}
};

#endif /* SESSIONFACTORY_H_ */

