//! \file test_controller.cpp
//!
//! \brief Controller unit-tests 
//!
//! \author Tron Kindseth (tron@rti.com)
//!
//! \date Created on: Feb 7, 2011

#include "../controller.h"
#include <string>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <limits.h>
#include "ndds_cpp.h"
#include "controlplane.h"

////////////////////////////////////////////////////////////////////////////////
//
//
//
//                           TEST INFRASTRUCTURE
//
//
//
////////////////////////////////////////////////////////////////////////////////

//! \class RemoteSession
//! 
//! \brief This class represents the SFs view of a state.
//!
//! A controller only interacts with a SF w.r.t to sessions. The SF manages
//! 0 or more SLs, each session leader is only responsible for a single session
class RemoteSession {
public:
    
    // A SCPSlave object detects new session created on the session plane. Thus,
    // it shall never create a session object. Rather, it is received from the
    // SCP.
    RemoteSession(SCPSlave *_sl,SCPEventNewSession *_session,LSCPMaster *_lscp,int domainId,int appId)
    {
        char did[32];
        char sid[32];
		// char pidStr[32];
        scp = (SCPSlaveObject*)(_session->GetSession());
        sl = _sl;
        lscp = _lscp;
        
        control = com::xvd::neuron::scp::ControlTypeSupport::create_data();
        state = com::xvd::neuron::scp::StateTypeSupport::create_data();
        event = com::xvd::neuron::scp::EventTypeSupport::create_data();
        metrics = com::xvd::neuron::scp::MetricsTypeSupport::create_data();
		
        // Make a copy of the create script. When the SL comes back with STANDBY,
        // send the original script
        pLscpControl = com::xvd::neuron::lscp::ControlTypeSupport::create_data();
        sprintf(pLscpControl->script,"%s",_session->GetData()->script);
        
        pLscpState = com::xvd::neuron::lscp::StateTypeSupport::create_data();
        pLscpEvent = com::xvd::neuron::lscp::EventTypeSupport::create_data();
        pLscpMetrics = com::xvd::neuron::lscp::MetricsTypeSupport::create_data();
        
        pLCSPMasterObject = _lscp->CreateMasterObject(scp->GetSessionId());
        sprintf(control->script,"Created session %d",scp->GetSessionId());
        
        //TODO: Don't go to the ready state, need to create the session
        //      leader first. When the SL is ready, we can switch to ready for this 
        //      session.
        //
        state->state = com::xvd::neuron::OBJECT_STATE_READY;
        scp->Send(state);
        
        localState = INIT;
        
        sprintf(did,"%d",domainId);
        sprintf(sid,"%d",scp->GetSessionId());

#if 0
		// This code can be refactored as it is no longer support because the
		// SF code now resides in a different place.
		//
        // Fork off the SL
        sessionLeader = fork();
        if (sessionLeader != 0)   
        {
            printf("Created SL: %d/%d\n",sessionLeader,scp->GetSessionId());
        } 
        else
        {
            sprintf(pidStr,"%d",appId);            
            execl("bin/sl",
                  "bin/sl",
                  "-appId",
                  pidStr,
                  "-domainId",
                  did,
                  "-sessionId",
                  sid,                  
                  (char *)0);
        }
#endif
    }
    
    ~RemoteSession()
    {
        state->state = com::xvd::neuron::OBJECT_STATE_DELETE;
        scp->Send(state);
		
        // Delete the SF->SL path
        lscp->DeleteMasterObject(pLCSPMasterObject);
        com::xvd::neuron::scp::ControlTypeSupport::delete_data(control);
        com::xvd::neuron::scp::StateTypeSupport::delete_data(state);
        com::xvd::neuron::scp::EventTypeSupport::delete_data(event);
        com::xvd::neuron::scp::MetricsTypeSupport::delete_data(metrics);        
        
        com::xvd::neuron::lscp::ControlTypeSupport::delete_data(pLscpControl);
        com::xvd::neuron::lscp::StateTypeSupport::delete_data(pLscpState);
        com::xvd::neuron::lscp::EventTypeSupport::delete_data(pLscpEvent);
        com::xvd::neuron::lscp::MetricsTypeSupport::delete_data(pLscpMetrics);                
		
		// Delete the SF->Controller path as the last thing we do
        sl->DeleteSlaveObject(scp);
    }
    
    // Received a session update from the SF
    void Update(com::xvd::neuron::scp::Control *control)
    {
        if (localState == UPDATE) 
        {
            printf("SF: Already updating, ignore\n");
            return;
        }
        
        // Change state, we don't accept more than on update
        localState = UPDATE;
        
        // Let the controller know the update has started
        state->state = com::xvd::neuron::OBJECT_STATE_UPDATE;
        scp->Send(state);
        
		if (0) 
		{
			// Send the update to the SL
			sprintf(pLscpControl->script,"%s",control->script);
			pLCSPMasterObject->Send(pLscpControl,sessionLeader);
		} 
		else
		{
			state->state = com::xvd::neuron::OBJECT_STATE_READY;
			scp->Send(state);
			localState = READY;
		}
    }
    
    void UpdateMetrics(void)
    {
        metrics->bytesSent += 100;
        metrics->bytesReceived += 134;
        metrics->entityCount += 100;
        scp->Send(metrics);        
    }
    
    int GetSessionId()
    {
        return scp->GetSessionId();
    }
    
    // Handle updates from the SL    
    void SLStateUpdate(com::xvd::neuron::lscp::State *lscp_state)
    {
        if (lscp_state->state == com::xvd::neuron::OBJECT_STATE_STANDBY)
        {
            pLCSPMasterObject->Send(pLscpControl,lscp_state->srcId); 
        } 
        else if (lscp_state->state == com::xvd::neuron::OBJECT_STATE_READY)
        {
            state->state = lscp_state->state;
            scp->Send(state);
            localState = READY;
        }            
    }
    
    
private:
    SCPSlave *sl;
    SCPSlaveObject *scp;
    LSCPMasterObject *pLCSPMasterObject;
    LSCPMaster *lscp;
    com::xvd::neuron::scp::Control *control;
    com::xvd::neuron::scp::State *state;
    com::xvd::neuron::scp::Event *event;
    com::xvd::neuron::scp::Metrics *metrics;
	
    com::xvd::neuron::lscp::Control *pLscpControl;
    com::xvd::neuron::lscp::State *pLscpState;
    com::xvd::neuron::lscp::Event *pLscpEvent;
    com::xvd::neuron::lscp::Metrics *pLscpMetrics;
    
    pid_t sessionLeader;
    
    enum 
    {
        INIT,
        STANDBY,
        READY,
        UPDATE,
    } localState;
};

class SessionFactory {
private:
    class SessionFactoryEventHandler : public EventHandlerT<SessionFactoryEventHandler>
    {
    public:
        com::xvd::neuron::acp::Control *pACcontrol;
        com::xvd::neuron::scp::Control *control;
        
        SessionFactory *sf;
        
        SessionFactoryEventHandler(SessionFactory *_sf) : EventHandlerT<SessionFactoryEventHandler>()
        { 
            sf = _sf;
            AddHandleFunc(&SessionFactoryEventHandler::MyEventHandler,ACP_EVENT_NEW_SESSION);
            AddHandleFunc(&SessionFactoryEventHandler::MyEventHandler,ACP_EVENT_UPDATE_SESSION);
            AddHandleFunc(&SessionFactoryEventHandler::MyEventHandler,ACP_EVENT_DELETE_SESSION);
            AddHandleFunc(&SessionFactoryEventHandler::MyEventHandler,SCP_EVENT_NEW_SESSION);
            AddHandleFunc(&SessionFactoryEventHandler::MyEventHandler,SCP_EVENT_UPDATE_SESSION);
            AddHandleFunc(&SessionFactoryEventHandler::MyEventHandler,SCP_EVENT_DELETE_SESSION);
            AddHandleFunc(&SessionFactoryEventHandler::MyEventHandler,LSCP_EVENT_SESSION_STATE_UPDATE);
            AddHandleFunc(&SessionFactoryEventHandler::MyEventHandler,LSCP_EVENT_SESSION_METRICS_UPDATE);
            AddHandleFunc(&SessionFactoryEventHandler::MyEventHandler,LSCP_EVENT_SESSION_EVENT);  
        }
        
        void MyEventHandler(Event *e) 
        {
            switch (e->GetKind()) {
                case ACP_EVENT_NEW_SESSION:
					pACcontrol = (reinterpret_cast<ACPEventNewSession*>(e))->GetData();
                    sf->HandleCommand(pACcontrol);
                    break;					
                case ACP_EVENT_UPDATE_SESSION:
                    pACcontrol = (reinterpret_cast<ACPEventUpdateSession*>(e))->GetData();
                    sf->HandleCommand(pACcontrol);
                    break;
                case ACP_EVENT_LOST_SESSION:
                    //sf->LostController((reinterpret_cast<ACPEventLostSession*>(e))->GetSessionId());
                    break;					
                case ACP_EVENT_DELETE_SESSION:
                    sf->Shutdown();
                    break;
					
					
                case SCP_EVENT_NEW_SESSION:
					{
                    SCPEventNewSession* se = reinterpret_cast<SCPEventNewSession*>(e);
                    sf->NewSession(se);
					}
                    break;
                case SCP_EVENT_LOST_SESSION:
                    break;					
                case SCP_EVENT_UPDATE_SESSION:
                    control = reinterpret_cast<SCPEventUpdateSession*>(e)->GetData();
                    sf->UpdateSession(control);
                    break;
                case SCP_EVENT_DELETE_SESSION:
                    sf->DeleteSession(reinterpret_cast<SCPEventDeleteSession*>(e)->GetSessionId());
                    break;
					
					
                case LSCP_EVENT_SESSION_STATE_UPDATE:
                    printf("LSCP_EVENT_SESSION_STATE_UPDATE\n");
                    sf->SLStateUpdate(reinterpret_cast<LSCPEventSessionStateUpdate*>(e)->GetData());
                    break;
                case LSCP_EVENT_SESSION_METRICS_UPDATE:
                    printf("LSCP_EVENT_SESSION_METRICS_UPDATE\n");
                    break;
                case LSCP_EVENT_SESSION_EVENT:
                    printf("LSCP_EVENT_SESSION_EVENT\n");
                    break;                
                default:
                    printf("Unknown event: %d\n",e->GetKind());
                    break;
            }
        }
        
        virtual void EventHandleLoop (void)
        {
        }    
    };
    
public:
    
    //! \brief Create a new session upon request from a controller
    void NewSession(SCPEventNewSession *newSession)
    {
        map<int,RemoteSession*>::iterator it;
        RemoteSession *session;
        
        int sessionId = ((SCPSlaveObject*)newSession->GetSession())->GetSessionId();
        
        it = rsessions.find(sessionId);
        if (it != rsessions.end())
        {
            PrintLog("Session %d already exists, bailing out...\n",sessionId);
        }        
		else 
		{
			PrintLog("Created session %d, SF=%x\n",sessionId,m_appId);
		}
        session = new RemoteSession(pSCPSlave,newSession,pLSCPMaster,m_domainId,m_appId);
        rsessions[sessionId]=session;
    }
    
    //! \brief Update a session upon request from a controller
    void UpdateSession(com::xvd::neuron::scp::Control *control)
    {
        map<int,RemoteSession*>::iterator it;
		char cmdbuf[128];
		
        PrintLog("Update session %d received from %d\n",control->sessionId,control->srcId);
        
		sprintf(cmdbuf,"delete session %d",control->sessionId);
		
		if (!strcmp(control->script,cmdbuf))
		{
			DeleteSession(control->sessionId);
		} 
		else
		{
			it = rsessions.find(control->sessionId);
			if (it != rsessions.end())
			{
				it->second->Update(control);
			}
		}
    }
	
    //! \brief Delete a session upon request from a controller
    void DeleteSession(int sessionId)
    {
        map<int,RemoteSession*>::iterator it;
		
        it = rsessions.find(sessionId);
        if (it != rsessions.end())
        {
            delete it->second;
            rsessions.erase(it->first);
        }
    }
    
    //! Handle received from a SL
    void SLStateUpdate(com::xvd::neuron::lscp::State *state)
    {
        RemoteSessionMap::iterator it;
        
        // We will get state from all SL created, also by other
        // SFs. Ignore those that are not managed by us. This is because
        // There is currently no good way of adding keys to a filter expression
        // other than using a custom filter, something which may be done in the 
        // future
        if (state->srcId != m_appId)
            return;
        
        it = rsessions.find(state->sessionId);
        
        if (it == rsessions.end()) 
        {
            PrintLog("Don't know anything about session %d\n",state->sessionId);
            return;
        }
        
        it->second->SLStateUpdate(state);
    }
    
    //! \brief Constructor for the SF. The SCP connects to multiple
    //! planes:
    //! ACP  - It is a slavem, being managed by a controller
    //! SCP  - It is a slave, responding to requests to create sessions, and
    //!        report state on sessions
    //! LSCP - It is a master of session leader, requesting session to be 
    //!        created, and listening on state changes on sessions
    //!
    SessionFactory(int masterId, int appId,int domainId)
    {
        m_appId = appId;
        m_domainId = domainId;
        doRun = true;
		inEvent = true;
		dtorsCalled = false;
		map<string,string> nvPairs;
		map<string,string>      PropertyPairsACP;
		map<string,DDS_Boolean> PropagateDiscoveryFlagsACP;
		map<string,string>      PropertyPairsSCP;
		map<string,DDS_Boolean> PropagateDiscoveryFlagsSCP;
        pACstate = com::xvd::neuron::acp::StateTypeSupport::create_data();
        pACevent = com::xvd::neuron::acp::EventTypeSupport::create_data();
        pACmetrics = com::xvd::neuron::acp::MetricsTypeSupport::create_data();
        
        pSessionFactoryEventHandler = new SessionFactoryEventHandler(this);
        
		PropertyPairsACP["CPInterfaceType"] = "ACP:Slave";
		PropagateDiscoveryFlagsACP["CPInterfaceType"] = DDS_BOOLEAN_TRUE;	
		PropertyPairsACP["Id"] = ToString<int>(appId);
		PropagateDiscoveryFlagsACP["Id"] = DDS_BOOLEAN_TRUE;	
		PropertyPairsACP["MasterId"] = ToString<int>(masterId);
		PropagateDiscoveryFlagsACP["MasterId"] = DDS_BOOLEAN_TRUE;	
		
        // The Controller serves as the admin master for all SFs. 
        pACPSlave = new ACPSlave(pSessionFactoryEventHandler,appId,domainId,"SF_ACPSlave",PropertyPairsACP,PropagateDiscoveryFlagsACP,"ACP");

        // The Controller manages sessions
		PropertyPairsACP["CPInterfaceType"] = "SCP:Slave";
    	pSCPSlave = new SCPSlave(pSessionFactoryEventHandler,appId,domainId,"SF_SCPSlave",PropertyPairsACP,PropagateDiscoveryFlagsACP,"SCP");
        
        // The Controller serves as the admin master for all SFs. 
        pLSCPMaster = new LSCPMaster(pSessionFactoryEventHandler,appId,domainId,"SF_LCSPMaster",PropertyPairsACP,PropagateDiscoveryFlagsACP,"LSCP");
		
        pACPSlaveObject = pACPSlave->CreateSlaveObject(appId);
        
        // By convention, a SF enters the admin plane in STANDBY mode
        pACstate->state = com::xvd::neuron::OBJECT_STATE_STANDBY;        
        pACPSlaveObject->Send(pACstate);
		
        // NOTE: This used RTI APIs for thread creation, replace with XVD thread
        //       creation
        eventThread = RTIOsapiThread_new("SFEventThread", 
                                         RTI_OSAPI_THREAD_PRIORITY_NORMAL, 
                                         0,
                                         PTHREAD_STACK_MIN*4, 
                                         NULL, 
                                         eventLoop, 
                                         this);
    }
	
    //! \brief Destructor for a SF
    //! 
    //! Change state to deleted. If this state change is lost
    //! a session controller will get a liveliness lost event on the state
    //! handle that. When the slave object is deleted, the state is disposed
    //! of. However, this is also sent as a message and could get lost
    //!
    ~SessionFactory()
    {
		dtorsCalled = true;
		doRun = false;
		while (inEvent)
		{
			usleep(10000);
		}
        pACstate->state = com::xvd::neuron::OBJECT_STATE_DELETE;        
        pACPSlaveObject->Send(pACstate);
        pACPSlave->DeleteSlaveObject(pACPSlaveObject);
        delete pSCPSlave;
        delete pACPSlave;
        delete pSessionFactoryEventHandler;
    }
    
    //! \brief A SF has detected us, so we enter the ready state
    //!
    //! It could be that a controller has rediscovered us, and thus
    //! pinged us. In that case the there is nothing to do
    void EnterReadyState()
    {
        if (pACstate->state ==  com::xvd::neuron::OBJECT_STATE_STANDBY)
        {
            pACstate->state = com::xvd::neuron::OBJECT_STATE_READY;
            pACPSlaveObject->Send(pACstate);
        }
    }
	
    //! \brief Shutdown the SF
    //!
    //! Exit the event-loop 
    void Shutdown(void)
    {
        PrintLog("Shutting down ....\n");
        doRun = false;
    }
	
    //! \brief Handle commands sent by the controller. 
    //! 
    //! Handling events in this function implies the the controller
    //! has discovered this SF
    void HandleCommand(com::xvd::neuron::acp::Control *control)
    {
        // The script is a just a string in this example.
        // This should be changed.
		PrintLog("Received script {%s}\n",control->script);
		if (strcmp(control->script,"ping"))
		{
			EnterReadyState();
		}
		
    }
    
    //! \brief Event-loop
    //!
    //! Function which handles incoming events
    static void* eventLoop(void *param)
    {
        SessionFactory *ev = (SessionFactory*)param;
		ev->inEvent = true;
        while (ev->doRun)
        {
            ev->pSessionFactoryEventHandler->HandleNextEvent();
			usleep(10000);
        }
		ev->inEvent = false;
		if (!ev->dtorsCalled)
		{
			delete ev;
		}
        return NULL;
    }
    
    //! \brief Print info about this SF
    void PrintInfo()
    {
    }
    
    void PrintLog(const char *logfmt,...)
    {
        va_list vl;
        va_start(vl,logfmt);
        printf("SF[%d@%d]:",m_appId,m_domainId);
        vprintf(logfmt,vl);
        va_end(vl);
    }
    
private:
    int m_appId;
    int m_domainId;
    bool doRun;
    bool inEvent;
	bool dtorsCalled;
	
    //! \var pACSlave
    //! \brief Attachment to the ACP plane, SF are managed
    ACPSlave *pACPSlave;
    
    //! \var pACSlaveObject
    //! \brief Only a single admin session ever exists   
    ACPSlaveObject *pACPSlaveObject;
    
    //! \var pSCPMaster 
    //! \brief Attachment to the SCP plane
    SCPSlave *pSCPSlave;
	
    //! \var pLSCPMaster 
    //! \brief Attachment to the LSCP plane
    LSCPMaster *pLSCPMaster;
	
    SessionFactoryEventHandler *pSessionFactoryEventHandler;
    
    com::xvd::neuron::acp::State *pACstate;
    com::xvd::neuron::acp::Event *pACevent;
    com::xvd::neuron::acp::Metrics *pACmetrics;
    
    RTIOsapiThread *eventThread;
    
    typedef std::map<int,RemoteSession*> RemoteSessionMap;
    typedef std::map<int,pid_t> SessionLeaderMap;
    
    RemoteSessionMap rsessions;
};
////////////////////////////////////////////////////////////////////////////////
//
//
//
//                         END TEST INFRASTRUCTURE
//
//
//
////////////////////////////////////////////////////////////////////////////////

//! \brief Test case 1: Single SF, test basic life-cycle
//!
//! This test executes the following steps:
//!
//! - Create SF
//! - Wait for STANDBY
//! - Send enable
//! - Wait for READY
//! - Send shutdown
//! - Wait for DELETE
//! - Wait for Deleted
class TestListener1 : public ControllerListener {
public:
	
	TestListener1()
	{
		onSFDetectCalls = 0;
		onSFStateChangeCalls = 0;
		onSFDeleteCalls = 0;
		onSFTerminateCalls = 0;
		onSFEventCalls = 0;
		onSFMetricUpdateCalls = 0;
	}
	
	virtual bool onSFDetect(Controller *c,int srcId) 
	{ 
		cout << "TESTLISTENER::onSFDetect" << endl;
		++onSFDetectCalls;
		return true; 
	}
	
	virtual bool onSFStateChange(Controller *c,int srcId,com::xvd::neuron::acp::State *state) 
	{ 
		char cmd[64];
		cout << "TESTLISTENER::onSFStateChange: " << srcId << "#" << state->state << " @" << endl;	
		++onSFStateChangeCalls;
		if (state->state == com::xvd::neuron::OBJECT_STATE_STANDBY)
		{
			sprintf(cmd,"acp enable %d",srcId);
			c->runSingleCommand(cmd);
		}
		return true; 
	}
	
	virtual bool onSFDelete(Controller *c,int srcId) 
	{ 
		cout << "TESTLISTENER::onSFDelete" << endl;
		++onSFDeleteCalls;
		return true; 
	}

	virtual bool onSFTerminate(Controller *c,int srcId) 
	{ 
		cout << "TESTLISTENER::onSFTerminate" << endl;
		++onSFTerminateCalls;
		return true; 
	}
	
	virtual void onSFEvent(Controller *c,int srcId) 
	{ 
		cout << "TESTLISTENER::onSFEvent" << endl;
		++onSFEventCalls;
		return; 
	}
		
	virtual void onSFMetricUpdate(Controller *c,int srcId) 
	{
		cout << "TESTLISTENER::onSFMetricUpdate" << endl;
		++onSFMetricUpdateCalls;
		return; 
	}
	
	int onSFDetectCalls;
	int onSFStateChangeCalls;
	int onSFDeleteCalls;
	int onSFTerminateCalls;
	int onSFEventCalls;
	int onSFMetricUpdateCalls;
};

int TestCase_1(int domainId,int appId)
{
	const char test_name[]="TestCase_1";
	int retval = 0;
	map<string,string> nvPairs;
	Controller *pTestCtrl = NULL;
	SessionFactory *sf;
	TestListener1 *testListener;
	char cmd[64];

	cout << "Executing test " << test_name << endl;
	
	nvPairs["use_lan_only"]="true";
	
	testListener = new TestListener1();
	pTestCtrl = new Controller(appId, domainId, nvPairs, testListener);
	
	if (pTestCtrl == NULL)
    {
		cout << "Failed to create Controller" << endl;
		++retval;
		goto done;
    }
	
	sf = new SessionFactory(appId,0xacdc,domainId);
	if (sf == NULL) 
	{
		cout << "Failed to create SF" << endl;
		++retval;
		goto done;
	}
	
	sleep(5);
	
	sprintf(cmd,"acp shutdown %d",0xacdc);
	
	pTestCtrl->runSingleCommand(cmd);

	sleep(5);

	if ((testListener->onSFDetectCalls != 1) ||
		(testListener->onSFStateChangeCalls != 3) ||
		(testListener->onSFDeleteCalls != 1)) 
	{
		++retval;
	}
	
	delete pTestCtrl;
	delete testListener;
	
done:
	return retval;
}

//! \brief Test case 2: Single SF, shutdown SF in standby state
//!
//! This test executes the following steps:
//!
//! - Create SF
//! - Wait for STANDBY
//! - Send shutdown
//! - Wait for DELETE
//! - Wait for Deleted
class TestListener2 : public ControllerListener {
public:
	
	TestListener2()
	{
		onSFDetectCalls = 0;
		onSFStateChangeCalls = 0;
		onSFDeleteCalls = 0;
		onSFTerminateCalls = 0;
		onSFEventCalls = 0;
		onSFMetricUpdateCalls = 0;
	}
	
	virtual bool onSFDetect(Controller *c,int srcId) 
	{ 
		cout << "TESTLISTENER::onSFDetect" << endl;
		++onSFDetectCalls;
		return true; 
	}
	
	virtual bool onSFStateChange(Controller *c,int srcId,com::xvd::neuron::acp::State *state) 
	{ 
		char cmd[64];
		cout << "TESTLISTENER::onSFStateChange: " << srcId << "#" << state->state << " @" << endl;	
		if (state->state == com::xvd::neuron::OBJECT_STATE_STANDBY)
		{
			sprintf(cmd,"acp ping %d",srcId);
			c->runSingleCommand(cmd);
		}		
		++onSFStateChangeCalls;
		return true; 
	}
	
	virtual bool onSFDelete(Controller *c,int srcId) 
	{ 
		cout << "TESTLISTENER::onSFDelete" << endl;
		++onSFDeleteCalls;
		return true; 
	}
	
	virtual bool onSFTerminate(Controller *c,int srcId) 
	{ 
		cout << "TESTLISTENER::onSFTerminate" << endl;
		++onSFTerminateCalls;
		return true; 
	}
	
	virtual void onSFEvent(Controller *c,int srcId) 
	{ 
		cout << "TESTLISTENER::onSFEvent" << endl;
		++onSFEventCalls;
		return; 
	}
	
	virtual void onSFMetricUpdate(Controller *c,int srcId) 
	{
		cout << "TESTLISTENER::onSFMetricUpdate" << endl;
		++onSFMetricUpdateCalls;
		return; 
	}
	
	int onSFDetectCalls;
	int onSFStateChangeCalls;
	int onSFDeleteCalls;
	int onSFTerminateCalls;
	int onSFEventCalls;
	int onSFMetricUpdateCalls;
};

int TestCase_2(int domainId,int appId)
{
	const char test_name[]="TestCase_2";
	int retval = 0;
	map<string,string> nvPairs;
	Controller *pTestCtrl = NULL;
	SessionFactory *sf;
	TestListener2 *testListener;
	char cmd[64];
	
	cout << "Executing test " << test_name << endl;
	
	nvPairs["use_lan_only"]="true";
	
	testListener = new TestListener2();
	pTestCtrl = new Controller(appId, domainId, nvPairs, testListener);
	
	if (pTestCtrl == NULL)
    {
		cout << "Failed to create Controller" << endl;
		++retval;
		goto done;
    }
	
	sf = new SessionFactory(appId,0xacdc,domainId);
	if (sf == NULL) 
	{
		cout << "Failed to create SF" << endl;
		++retval;
		goto done;
	}
	
	sleep(5);
	
	sprintf(cmd,"acp shutdown %d",0xacdc);
	
	pTestCtrl->runSingleCommand(cmd);
	
	sleep(5);
	
	if ((testListener->onSFDetectCalls != 1) ||
		(testListener->onSFStateChangeCalls != 2) ||
		(testListener->onSFDeleteCalls != 1)) 
	{
		++retval;
	}
	
	delete pTestCtrl;
	delete testListener;
	
done:
	return retval;
}

//! \brief Test case 3: Single SF, Terminate created SF in READY state
//!
//! This test executes the following steps:
//!
//! - Create SF
//! - Wait for STANDBY
//! - Send enable
//! - Wait for READY
//! - Terminate SF
class TestListener3 : public TestListener1 {
};

int TestCase_3(int domainId,int appId)
{
	const char test_name[]="TestCase_3";
	int retval = 0;
	map<string,string> nvPairs;
	Controller *pTestCtrl = NULL;
	SessionFactory *sf;
	TestListener3 *testListener;
	
	cout << "Executing test " << test_name << endl;
	
	nvPairs["use_lan_only"]="true";
	
	testListener = new TestListener3();
	pTestCtrl = new Controller(appId, domainId, nvPairs, testListener);
	
	if (pTestCtrl == NULL)
    {
		cout << "Failed to create Controller" << endl;
		++retval;
		goto done;
    }
	
	sf = new SessionFactory(appId,0xacdc,domainId);
	if (sf == NULL) 
	{
		cout << "Failed to create SF" << endl;
		++retval;
		goto done;
	}
	
	sleep(5);
	
	delete sf;
	
	sleep(5);
	
	if ((testListener->onSFDetectCalls != 1) ||
		(testListener->onSFStateChangeCalls != 3) ||
		(testListener->onSFTerminateCalls != 1) ||
		(testListener->onSFDeleteCalls != 0))
	{
		++retval;
	}
	
	delete pTestCtrl;
	delete testListener;
	
done:
	return retval;
}

//! \brief Test case 4: Single SF, Terminate created SF in STANDBY state
//!
//! This test executes the following steps:
//!
//! - Create SF
//! - Wait for STANDBY
//! - Send enable
//! - Wait for READY
//! - Terminate SF
class TestListener4 : public TestListener2 {
};

int TestCase_4(int domainId,int appId)
{
	const char test_name[]="TestCase_4";
	int retval = 0;
	map<string,string> nvPairs;
	Controller *pTestCtrl = NULL;
	SessionFactory *sf;
	TestListener4 *testListener;
	
	cout << "Executing test " << test_name << endl;
	
	nvPairs["use_lan_only"]="true";
	
	testListener = new TestListener4();
	pTestCtrl = new Controller(appId, domainId, nvPairs, testListener);
	
	if (pTestCtrl == NULL)
    {
		cout << "Failed to create Controller" << endl;
		++retval;
		goto done;
    }
	
	sf = new SessionFactory(appId,0xacdc,domainId);
	if (sf == NULL) 
	{
		cout << "Failed to create SF" << endl;
		++retval;
		goto done;
	}
	
	sleep(5);
	
	delete sf;
	
	sleep(5);
	
	if ((testListener->onSFDetectCalls != 1) ||
		(testListener->onSFStateChangeCalls != 2) ||
		(testListener->onSFTerminateCalls != 1) ||
		(testListener->onSFDeleteCalls != 0))
	{
		++retval;
	}
	
	delete pTestCtrl;
	delete testListener;
	
done:
	return retval;
}

//! \brief Test case 5: Multiple SFs, test basic life-cycle
//!
//! This test executes the following steps:
//!
//! - Create SF
//! - Wait for STANDBY
//! - Send enable
//! - Wait for READY
//! - Send shutdown
//! - Wait for DELETE
//! - Wait for Deleted
class TestListener5 : public TestListener1 {
};

int TestCase_5(int domainId,int appId)
{
#define MAX_SF 5
	const char test_name[]="TestCase_5";
	int retval = 0;
	map<string,string> nvPairs;
	Controller *pTestCtrl = NULL;
	SessionFactory *sf[MAX_SF];
	TestListener1 *testListener;
	char cmd[64];
	int i;
	
	cout << "Executing test " << test_name << endl;
	
	nvPairs["use_lan_only"]="true";
	
	testListener = new TestListener1();
	pTestCtrl = new Controller(appId, domainId, nvPairs, testListener);
	
	if (pTestCtrl == NULL)
    {
		cout << "Failed to create Controller" << endl;
		++retval;
		goto done;
    }
	
	for (i = 0; i < MAX_SF; ++i) 
	{
		sf[i] = new SessionFactory(appId,0xacdc+i,domainId);
		if (sf[i] == NULL) 
		{
			cout << "Failed to create SF" << endl;
			++retval;
			goto done;
		}
	}
	
	sleep(5);
	
	// Shutdown all SFs using different versions of the command
	sprintf(cmd,"acp shutdown %d",0xacdc);
	pTestCtrl->runSingleCommand(cmd);

	sprintf(cmd,"acp shutdown %d %d %d",0xacdc+1,0xacdc+2,0xacdc+3);
	pTestCtrl->runSingleCommand(cmd);

	sprintf(cmd,"acp shutdown %d",0xacdc+4);
	pTestCtrl->runSingleCommand(cmd);
	
	sleep(5);
	
	if ((testListener->onSFDetectCalls != 1*MAX_SF) ||
		(testListener->onSFStateChangeCalls != 3*MAX_SF) ||
		(testListener->onSFDeleteCalls != 1*MAX_SF)) 
	{
		++retval;
	}
	
	delete pTestCtrl;
	delete testListener;
	
done:
	return retval;
#undef MAX_SF
}

//! \brief Test case 5: Shutdown SFs in standby state
//!
//! This test executes the following steps:
//!
//! - Create SF
//! - Wait for STANDBY
//! - Send shutdown
//! - Wait for DELETE
//! - Wait for Deleted
class TestListener6 : public TestListener2 {
};

int TestCase_6(int domainId,int appId)
{
#define MAX_SF 5
	const char test_name[]="TestCase_6";
	int retval = 0;
	map<string,string> nvPairs;
	Controller *pTestCtrl = NULL;
	SessionFactory *sf[MAX_SF];
	TestListener2 *testListener;
	char cmd[64];
	int i;
	
	cout << "Executing test " << test_name << endl;
	
	nvPairs["use_lan_only"]="true";
	
	testListener = new TestListener2();
	pTestCtrl = new Controller(appId, domainId, nvPairs, testListener);
	
	if (pTestCtrl == NULL)
    {
		cout << "Failed to create Controller" << endl;
		++retval;
		goto done;
    }
	
	for (i = 0; i < MAX_SF; ++i) 
	{
		sf[i] = new SessionFactory(appId,0xacdc+i,domainId);
		if (sf[i] == NULL) 
		{
			cout << "Failed to create SF" << endl;
			++retval;
			goto done;
		}
	}
	
	sleep(5);
	
	// Shutdown all SFs using different versions of the command
	sprintf(cmd,"acp shutdown %d",0xacdc);
	pTestCtrl->runSingleCommand(cmd);
	
	sprintf(cmd,"acp shutdown %d %d %d",0xacdc+1,0xacdc+2,0xacdc+3);
	pTestCtrl->runSingleCommand(cmd);
	
	sprintf(cmd,"acp shutdown %d",0xacdc+4);
	pTestCtrl->runSingleCommand(cmd);
	
	sleep(5);
	
	if ((testListener->onSFDetectCalls != 1*MAX_SF) ||
		(testListener->onSFStateChangeCalls != 2*MAX_SF) ||
		(testListener->onSFDeleteCalls != 1*MAX_SF)) 
	{
		++retval;
	}
	
	delete pTestCtrl;
	delete testListener;
	
done:
	return retval;
#undef MAX_SF
}

//! \brief Test case 3: Single SF, Terminate created SF in READY state
//!
//! This test executes the following steps:
//!
//! - Create SF
//! - Wait for STANDBY
//! - Send enable
//! - Wait for READY
//! - Terminate SF
class TestListener7 : public TestListener1 {
};

int TestCase_7(int domainId,int appId)
{
#define MAX_SF 5
	const char test_name[]="TestCase_7";
	int retval = 0;
	map<string,string> nvPairs;
	Controller *pTestCtrl = NULL;
	SessionFactory *sf[MAX_SF];
	TestListener7 *testListener;
	int i;
	
	cout << "Executing test " << test_name << endl;
	
	nvPairs["use_lan_only"]="true";
	
	testListener = new TestListener7();
	pTestCtrl = new Controller(appId, domainId, nvPairs, testListener);
	
	if (pTestCtrl == NULL)
    {
		cout << "Failed to create Controller" << endl;
		++retval;
		goto done;
    }
	
	for (i = 0; i < MAX_SF; ++i) 
	{
		sf[i] = new SessionFactory(appId,0xacdc+i,domainId);
		if (sf[i] == NULL) 
		{
			cout << "Failed to create SF" << endl;
			++retval;
			goto done;
		}
	}
	
	sleep(5);
	
	for (i = 0; i < MAX_SF; ++i) 
	{
		delete sf[i];
	}
	
	sleep(5);
	
	if ((testListener->onSFDetectCalls != 1*MAX_SF) ||
		(testListener->onSFStateChangeCalls != 3*MAX_SF) ||
		(testListener->onSFTerminateCalls != 1*MAX_SF) ||
		(testListener->onSFDeleteCalls != 0))
	{
		++retval;
	}
	
	delete pTestCtrl;
	delete testListener;
	
done:
	return retval;
}

//! \brief Test case 4: Single SF, Terminate created SF in STANDBY state
//!
//! This test executes the following steps:
//!
//! - Create SF
//! - Wait for STANDBY
//! - Send enable
//! - Wait for READY
//! - Terminate SF
class TestListener8 : public TestListener2 {
};

int TestCase_8(int domainId,int appId)
{
#define MAX_SF 5
	const char test_name[]="TestCase_8";
	int retval = 0;
	map<string,string> nvPairs;
	Controller *pTestCtrl = NULL;
	SessionFactory *sf[MAX_SF];
	TestListener8 *testListener;
	int i;
	
	cout << "Executing test " << test_name << endl;
	
	nvPairs["use_lan_only"]="true";
	
	testListener = new TestListener8();
	pTestCtrl = new Controller(appId, domainId, nvPairs, testListener);
	
	if (pTestCtrl == NULL)
    {
		cout << "Failed to create Controller" << endl;
		++retval;
		goto done;
    }
	
	for (i = 0; i < MAX_SF; ++i) 
	{
		sf[i] = new SessionFactory(appId,0xacdc+i,domainId);
		if (sf[i] == NULL) 
		{
			cout << "Failed to create SF" << endl;
			++retval;
			goto done;
		}
	}
	
	sleep(5);
	
	for (i = 0; i < MAX_SF; ++i) 
	{
		delete sf[i];
	}
	
	sleep(5);
	
	if ((testListener->onSFDetectCalls != 1*MAX_SF) ||
		(testListener->onSFStateChangeCalls != 2*MAX_SF) ||
		(testListener->onSFTerminateCalls != 1*MAX_SF) ||
		(testListener->onSFDeleteCalls != 0*MAX_SF))
	{
		++retval;
	}
	
	delete pTestCtrl;
	delete testListener;
	
done:
	return retval;
#undef MAX_SF
}

////////////////////////////////////////////////////////////////////////////////
//  SESSION TESTS
////////////////////////////////////////////////////////////////////////////////


//! \brief Test case 9: Single SF, single session
//!
//! This test executes the following steps:
//!
//! - Create SF
//! - Wait for STANDBY
//! - Send enable
//! - Wait for READY
//! - Create session
//! - Wait for ready on SCP
//! - Delete session from SCP
//! - Wait for delete from SCP
//! - Wait for DELETED from SCP
class TestListener9 : public ControllerListener {
public:
	
	TestListener9()
	{
		onSFDetectCalls = 0;
		onSFStateChangeCalls = 0;
		onSFDeleteCalls = 0;
		onSFTerminateCalls = 0;
		onSFEventCalls = 0;
		onSFMetricUpdateCalls = 0;
		onSessionDetectCalls = 0;
		onSessionDeleteCalls = 0;
		onSFSessionStateChangeCalls = 0;
		onSFSessionDeleteCalls = 0;
		onSFSessionTerminateCalls = 0;		
	}
	
	virtual bool onSFDetect(Controller *c,int srcId) 
	{ 
		cout << "TESTLISTENER::onSFDetect" << endl;
		++onSFDetectCalls;
		return true; 
	}
	
	virtual bool onSFStateChange(Controller *c,int srcId,com::xvd::neuron::acp::State *state) 
	{ 
		char cmd[64];
		cout << "TESTLISTENER::onSFStateChange: " << srcId << "#" << state->state << " @" << endl;	
		++onSFStateChangeCalls;
		if (state->state == com::xvd::neuron::OBJECT_STATE_STANDBY)
		{
			sprintf(cmd,"acp enable %d",srcId);
			c->runSingleCommand(cmd);
		}
		return true; 
	}
	
	virtual bool onSFDelete(Controller *c,int srcId) 
	{ 
		cout << "TESTLISTENER::onSFDelete" << endl;
		++onSFDeleteCalls;
		return true; 
	}
	
	virtual bool onSFTerminate(Controller *c,int srcId) 
	{ 
		cout << "TESTLISTENER::onSFTerminate" << endl;
		++onSFTerminateCalls;
		return true; 
	}
	
	virtual void onSFEvent(Controller *c,int srcId) 
	{ 
		cout << "TESTLISTENER::onSFEvent" << endl;
		++onSFEventCalls;
		return; 
	}
	
	virtual void onSFMetricUpdate(Controller *c,int srcId) 
	{
		cout << "TESTLISTENER::onSFMetricUpdate" << endl;
		++onSFMetricUpdateCalls;
		return; 
	}
		
	// ------- Session Listeners -------

	virtual bool onSessionDetect(Controller *c,int sessionId) 
	{
		cout << "TESTLISTENER::onSessionDetect" << endl;
		++onSessionDetectCalls;
		return true; 
	}

	virtual bool onSessionDelete(Controller *c,int sessionId) 
	{
		cout << "TESTLISTENER::onSessionDelete" << endl;
		++onSessionDeleteCalls;
		return true; 
	}
	
	virtual bool onSFSessionStateChange(Controller *c,
										int sessionId, 
										int srcId,
										com::xvd::neuron::scp::State *state) 
	{ 
		cout << "TESTLISTENER::onSFSessionStateChange: session=" << sessionId 
			<< " src=" << srcId 
			<< " state=" << state->state << endl;
		++onSFSessionStateChangeCalls;
		return true; 
	}
	
	virtual bool onSFSessionDelete(Controller *c,int sessionId,int srcId) 
	{ 
		cout << "TESTLISTENER::onSFSessionDelete" << endl;
		++onSFSessionDeleteCalls;
		return true; 
	}

	virtual bool onSFSessionTerminate(Controller *c,int sessionId,int srcId) 
	{ 
		cout << "TESTLISTENER::onSFSessionTerminate" << endl;
		++onSFSessionTerminateCalls;
		return true; 
	}

	int onSFDetectCalls;
	int onSFStateChangeCalls;
	int onSFDeleteCalls;
	int onSFTerminateCalls;
	int onSFEventCalls;
	int onSFMetricUpdateCalls;
	
	int onSessionDetectCalls;
	int onSessionDeleteCalls;
	int onSFSessionStateChangeCalls;
	int onSFSessionDeleteCalls;
	int onSFSessionTerminateCalls;
};

int TestCase_9(int domainId,int appId)
{
	const char test_name[]="TestCase_9";
	int retval = 0;
	map<string,string> nvPairs;
	Controller *pTestCtrl = NULL;
	SessionFactory *sf;
	TestListener9 *testListener;
	
	cout << "Executing test " << test_name << endl;
	
	nvPairs["use_lan_only"]="true";
	
	testListener = new TestListener9();
	pTestCtrl = new Controller(appId, domainId, nvPairs, testListener);
	
	if (pTestCtrl == NULL)
    {
		cout << "Failed to create Controller" << endl;
		++retval;
		goto done;
    }
	
	sf = new SessionFactory(appId,0xacdc,domainId);
	if (sf == NULL) 
	{
		cout << "Failed to create SF" << endl;
		++retval;
		goto done;
	}
	
	sleep(5);
	
	pTestCtrl->runSingleCommand("scp create 1000 0xacdc");
	
	sleep(5);
	
	pTestCtrl->runSingleCommand("scp delete 1000");

	sleep(5);

	pTestCtrl->runSingleCommand("acp shutdown 0xacdc");

	sleep(5);

	if ((testListener->onSFDetectCalls != 1) ||
		(testListener->onSFStateChangeCalls != 3) ||
		(testListener->onSFDeleteCalls != 1) ||
		(testListener->onSessionDetectCalls != 0) ||
		(testListener->onSFSessionStateChangeCalls != 2) ||
		(testListener->onSFSessionDeleteCalls != 1) ||
		(testListener->onSessionDeleteCalls != 1))
		
	{
		++retval;
	}
	
	delete pTestCtrl;
	delete testListener;
	
done:
	return retval;
}

class TestListener10 : public TestListener9 {
};

int TestCase_10(int domainId,int appId)
{
	const char test_name[]="TestCase_10";
	int retval = 0;
	map<string,string> nvPairs;
	Controller *pTestCtrl = NULL;
	SessionFactory *sf;
	TestListener10 *testListener;
	
	cout << "Executing test " << test_name << endl;
	
	nvPairs["use_lan_only"]="true";
	
	testListener = new TestListener10();
	pTestCtrl = new Controller(appId, domainId, nvPairs, testListener);
	
	if (pTestCtrl == NULL)
    {
		cout << "Failed to create Controller" << endl;
		++retval;
		goto done;
    }
	
	sf = new SessionFactory(appId,0xacdc,domainId);
	if (sf == NULL) 
	{
		cout << "Failed to create SF" << endl;
		++retval;
		goto done;
	}
	
	sleep(5);
	
	pTestCtrl->runSingleCommand("scp create 1000 0xacdc");
	
	sleep(5);
	
	pTestCtrl->runSingleCommand("scp delete 1000 0xacdc");
	
	sleep(5);

	pTestCtrl->runSingleCommand("scp delete 1000");
	
	sleep(5);
	
	pTestCtrl->runSingleCommand("acp shutdown 0xacdc");
	
	sleep(5);
	
	if ((testListener->onSFDetectCalls != 1) ||
		(testListener->onSFStateChangeCalls != 3) ||
		(testListener->onSFDeleteCalls != 1) ||
		(testListener->onSessionDetectCalls != 0) ||
		(testListener->onSFSessionStateChangeCalls != 2) ||
		(testListener->onSFSessionDeleteCalls != 1) ||
		(testListener->onSessionDeleteCalls != 1))
		
	{
		++retval;
	}
	
	delete pTestCtrl;
	delete testListener;
	
done:
	return retval;
}

class TestListener11 : public TestListener9 {
};

int TestCase_11(int domainId,int appId)
{
#define MAX_SF 3
	const char test_name[]="TestCase_11";
	int retval = 0;
	map<string,string> nvPairs;
	Controller *pTestCtrl = NULL;
	SessionFactory *sf[MAX_SF];
	TestListener10 *testListener;
	
	cout << "Executing test " << test_name << endl;
	
	nvPairs["use_lan_only"]="true";
	
	testListener = new TestListener10();
	pTestCtrl = new Controller(appId, domainId, nvPairs, testListener);
	
	if (pTestCtrl == NULL)
    {
		cout << "Failed to create Controller" << endl;
		++retval;
		goto done;
    }
	
	for (int i = 0; i < MAX_SF; ++i) 
	{
		sf[i] = new SessionFactory(appId,0xacdc+i,domainId);
		if (sf[i] == NULL) 
		{
			cout << "Failed to create SF" << endl;
			++retval;
			goto done;
		}
	}
	
	sleep(5);
	
	pTestCtrl->runSingleCommand("scp create 1000 0xacdc 0xacdd");
	
	sleep(5);
	
	pTestCtrl->runSingleCommand("scp delete 1000 0xacdc");
	
	sleep(5);
	
	pTestCtrl->runSingleCommand("scp delete 1000");
	
	sleep(5);
	
	pTestCtrl->runSingleCommand("acp shutdown 0xacdc 0xacdd 0xacde");
	
	sleep(5);
	
	if ((testListener->onSFDetectCalls != 1*MAX_SF) ||
		(testListener->onSFStateChangeCalls != 3*MAX_SF) ||
		(testListener->onSFDeleteCalls != 1*MAX_SF) ||
		(testListener->onSessionDetectCalls != 0) ||
		(testListener->onSFSessionStateChangeCalls != 2*2) ||
		(testListener->onSFSessionDeleteCalls != 1*2) ||
		(testListener->onSessionDeleteCalls != 1))
		
	{
		++retval;
	}
	
	delete pTestCtrl;
	delete testListener;
	
done:
	return retval;
}

int
main(int argc, char **argv)
{
    int domainId = 4; 
    int testAppId = 0;
    int i;
    DDSDomainParticipantFactory *factory = DDSDomainParticipantFactory::get_instance();
	int errors = 0;
	DDS_DomainParticipantFactoryQos fqos;
		
	factory->get_qos(fqos);
	fqos.profile.ignore_environment_profile = DDS_BOOLEAN_TRUE;
	fqos.profile.ignore_resource_profile = DDS_BOOLEAN_TRUE;
    fqos.resource_limits.max_objects_per_thread = 8192;
    factory->set_qos(fqos);
	
	for (i = 0; i < argc; ++i)
    {
        if (!strcmp("-testAppId",argv[i]))
        {
            ++i;
            if (i == argc)
            {
                printf("-testAppId <testAppId>\n");
                break;
            }
            testAppId = strtol(argv[i],NULL,0);
        }
        else if (!strcmp("-domainId",argv[i]))
        {
            ++i;
            if (i == argc)
            {
                printf("-domainId <domainId>\n");
                break;
            }
            domainId = strtol(argv[i],NULL,0);
        }
        else if (!strcmp("-help",argv[i]))
        {
			cout << "-domainId <domain id>" << endl;
			cout << "-testAppId <app id>" << endl;
			cout << "-help" << endl;
			cout << "-verbose" << endl;
        }
		else if (!strcmp("-verbose",argv[i]))
		{
			NDDSConfigLogger::get_instance()->set_verbosity_by_category(NDDS_CONFIG_LOG_CATEGORY_API, 
																		NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
			
			NDDSConfigLogger::get_instance()->set_verbosity_by_category(NDDS_CONFIG_LOG_CATEGORY_PLATFORM , 
																		NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
			
			NDDSConfigLogger::get_instance()->set_verbosity_by_category(NDDS_CONFIG_LOG_CATEGORY_COMMUNICATION , 
																		NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
			
			NDDSConfigLogger::get_instance()->set_verbosity_by_category(NDDS_CONFIG_LOG_CATEGORY_DATABASE , 
																		NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
			
			NDDSConfigLogger::get_instance()->set_verbosity_by_category(NDDS_CONFIG_LOG_CATEGORY_ENTITIES , 
																		NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
		}		
    }        
    
    if (i < argc) {
        return -1;
    }
	
	if (domainId < 0) 
	{
		cout << "domainId must be >= 0" << endl;
		return -1;
	}
	
	if (testAppId < 0) 
	{
		cout << "testAppId must be >= 0" << endl;
		return -1;
	}
		
	// ACP tests
	if (0) {
	errors += TestCase_1(domainId,testAppId);
	errors += TestCase_2(domainId,testAppId);
	errors += TestCase_3(domainId,testAppId);
	errors += TestCase_4(domainId,testAppId);
	errors += TestCase_5(domainId,testAppId);
	errors += TestCase_6(domainId,testAppId);
	errors += TestCase_7(domainId,testAppId);
	errors += TestCase_8(domainId,testAppId);
	
	// SCP test		
	errors += TestCase_9(domainId,testAppId);
	errors += TestCase_10(domainId,testAppId);
	}
	errors += TestCase_11(domainId,testAppId);

	if (errors != 0) 
	{
		cout << "Test result: FAILED (" << errors << " errors)" << endl;
	}
	else
	{
		cout << "Test result: PASSED" << endl;;
	}
	return errors;
}
