//!
//! \file sf.cpp
//!
//! \brief An example controller
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
//! The purpose of this example is to illustrate how an SF may function 
//!
//! Specifically. this code is provided "as is", without warranty, expressed
//! or implied.
//!
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "ndds_cpp.h"
#include "controlplane.h"

//! \class RemoteSession
//! 
//! \brief This class represents the SFs view of a state.
//!
//! A controller only interacts with a SF w.r.t to sessions. The SF manages
//! 0 or more SLs, each session leader is only responsible for a single session
//!
class RemoteSession 
{
public:
    
    // A SCPSlave object detects new session created on the session plane. Thus,
    // it shall never create a session object. Rather, it is received from the
    // SCP.
    RemoteSession(SCPSlave *_sl,SCPEventNewSession *_session,LSCPMaster *_lscp,int domainId,int appId)
    {
        char did[32];
        char sid[32];
        char pidStr[32];
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
        state->state = com::xvd::neuron::OBJECT_STATE_INIT;
        scp->Send(state);
        
        localState = INIT;
        
        sprintf(did,"%d",domainId);
        sprintf(sid,"%d",scp->GetSessionId());
        
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
    }
    
    ~RemoteSession()
    {
        state->state = com::xvd::neuron::OBJECT_STATE_DELETED;
        scp->Send(state);
        // Delete the SF->Controller path
        sl->DeleteSlaveObject(scp);

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
        
        // Send the update to the SL
        sprintf(pLscpControl->script,"%s",control->script);
        pLCSPMasterObject->Send(pLscpControl,sessionLeader);
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
                    sf->EnterReadyState();
                    break;
                case ACP_EVENT_UPDATE_SESSION:
                    pACcontrol = (reinterpret_cast<ACPEventUpdateSession*>(e))->GetData();
                    sf->HandleCommand(pACcontrol);
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
                case SCP_EVENT_UPDATE_SESSION:
                    control = reinterpret_cast<SCPEventUpdateSession*>(e)->GetData();
                    sf->UpdateSession(control);
                    break;
                case SCP_EVENT_DELETE_SESSION:
                    sf->DeleteSession(reinterpret_cast<SCPEventDeleteSession*>(e)->GetSessionId());
                    break;
                case LSCP_EVENT_SESSION_STATE_UPDATE:
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
        session = new RemoteSession(pSCPSlave,newSession,pLSCPMaster,m_domainId,m_appId);
        rsessions[sessionId]=session;
    }
    
    //! \brief Update a session upon request from a controller
    void UpdateSession(com::xvd::neuron::scp::Control *control)
    {
        map<int,RemoteSession*>::iterator it;
        PrintLog("Update session %d\n",control->sessionId);
        
        it = rsessions.find(control->sessionId);
        if (it != rsessions.end())
        {
            it->second->Update(control);
        }
    }
     
    //! \brief Delete a session upon request from a controller
    void DeleteSession(int sessionId)
    {
        map<int,RemoteSession*>::iterator it;
        PrintLog("Delete session: %d\n",sessionId);

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
    SessionFactory(int appId,int domainId)
    {
        m_appId = appId;
        m_domainId = domainId;
        doRun = true;
        
        pACstate = com::xvd::neuron::acp::StateTypeSupport::create_data();
        pACevent = com::xvd::neuron::acp::EventTypeSupport::create_data();
        pACmetrics = com::xvd::neuron::acp::MetricsTypeSupport::create_data();
        
        pSessionFactoryEventHandler = new SessionFactoryEventHandler(this);
        
        // The Controller manages sessions
    	pSCPSlave = new SCPSlave(pSessionFactoryEventHandler,appId,domainId,"SF_SCPSlave","SCP");
        
        // The Controller serves as the admin master for all SFs. 
        pACPSlave = new ACPSlave(pSessionFactoryEventHandler,appId,domainId,"SF_ACPSlave","ACP");

        // The Controller serves as the admin master for all SFs. 
        pLSCPMaster = new LSCPMaster(pSessionFactoryEventHandler,appId,domainId,"SF_LCSPMaster","LSCP");

        pACPSlaveObject = pACPSlave->CreateSlaveObject(appId);
        
        // By convention, a SF enters the admin plane in STANDBY mode
        pACstate->state = com::xvd::neuron::OBJECT_STATE_STANDBY;        
        pACPSlaveObject->Send(pACstate);

        // NOTE: This used RTI APIs for thread creation, replace with XVD thread
        //       creation
        eventThread = RTIOsapiThread_new("SFeventThread", 
                                         RTI_OSAPI_THREAD_PRIORITY_NORMAL, 
                                         0,
                                         PTHREAD_STACK_MIN*4, 
                                         NULL, 
                                         eventLoop, 
                                         this);
        while (doRun)
        {
            usleep(1000000);
        }
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
        pACstate->state = com::xvd::neuron::OBJECT_STATE_DELETED;        
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
    }
    
    //! \brief Event-loop
    //!
    //! Function which handles incoming events
    static void* eventLoop(void *param)
    {
        SessionFactory *ev = (SessionFactory*)param;
        while (ev->doRun)
        {
            usleep(100000);
            ev->pSessionFactoryEventHandler->HandleNextEvent();
        }
        delete ev;
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

int 
main(int argc, char **argv)
{
    int domainId = 67;
    int appId = 0;
	SessionFactory *tester = NULL;
    int i;
    DDSDomainParticipantFactory *factory = DDSDomainParticipantFactory::get_instance();
    
    DDS_DomainParticipantFactoryQos fqos;
    
    factory->get_qos(fqos);
    fqos.resource_limits.max_objects_per_thread = 8192;
    factory->set_qos(fqos);
    
    for (i = 0; i < argc; ++i)
    {
        if (!strcmp("-appId",argv[i]))
        {
            ++i;
            if (i == argc)
            {
                printf("-appId <appId>\n");
                break;
            }
            appId = strtol(argv[i],NULL,0);
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
        else if (!strcmp("-appId",argv[i]))
        {
            ++i;
            if (i == argc)
            {
                printf("-appId <appId>\n");
                break;
            }
        }
    }        
    
    if (i < argc) {
        printf("SF[%d/%d]: Error kicking off SF\n",appId,domainId);
        return -1;
    }
    
    //! This the constructor does not return until the SF terminates
    tester = new SessionFactory(appId,domainId);
    
    delete tester;
    
	return 0;
}

