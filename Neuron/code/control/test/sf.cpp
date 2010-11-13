#include <stdio.h>
#include <stdlib.h>
#include "ndds_cpp.h"
#include "controlplane.h"

class RemoteSession 
{
public:
    
    // A SCPSlave object detects new session created on the session plane. Thus,
    // it shall never create a session object. Rather, it is received from the
    // SCP.
    RemoteSession(SCPSlave *_sl,SCPSlaveObject *_scp,LSCPMaster *_lscp,int domainId)
    {
        scp = _scp;
        sl = _sl;
        lscp = _lscp;
        
        control = com::xvd::neuron::scp::ControlTypeSupport::create_data();
        state = com::xvd::neuron::scp::StateTypeSupport::create_data();
        event = com::xvd::neuron::scp::EventTypeSupport::create_data();
        metrics = com::xvd::neuron::scp::MetricsTypeSupport::create_data();

        pLscpControl = com::xvd::neuron::lscp::ControlTypeSupport::create_data();
        pLscpState = com::xvd::neuron::lscp::StateTypeSupport::create_data();
        pLscpEvent = com::xvd::neuron::lscp::EventTypeSupport::create_data();
        pLscpMetrics = com::xvd::neuron::lscp::MetricsTypeSupport::create_data();
        
        sessionLeader = new SessionLeader(_scp->GetSessionId(),domainId,_scp->GetSessionId());
        pLCSPMasterObject = _lscp->CreateMasterObject(_scp->GetSessionId());
        sprintf(control->script,"Created session %d",scp->GetSessionId());
        
        //TODO: Don't go to the ready state, need to create the session
        //      leader first. When the SL is ready, we can switch to ready for this 
        //      session.
        //
        state->state = com::xvd::neuron::OBJECT_STATE_INIT;
        scp->Send(state);
        
        sprintf(pLscpControl->script,"SL: create session with src->relay->sink\n");
        pLCSPMasterObject->Send(pLscpControl,_scp->GetSessionId());
    }
    
    ~RemoteSession()
    {
        sl->DeleteSlaveObject(scp);
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
    
    void Update(com::xvd::neuron::scp::Control *control)
    {
        // Let all know we are updating
        state->state = com::xvd::neuron::OBJECT_STATE_UPDATE;
        scp->Send(state);
        
        // Update
        printf("add script for session(%d): %s\n",
               scp->GetSessionId(),control->script);
        
        // Let all know we are done with the update
        state->state = com::xvd::neuron::OBJECT_STATE_READY;
        scp->Send(state);
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
    
    void SLStateUpdate(com::xvd::neuron::lscp::State *lscp_state)
    {
        printf("SL updated session %d\n",lscp_state->sessionId);
        // Let all know we are updating
        state->state = lscp_state->state;
        scp->Send(state);        
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
    
    SessionLeader *sessionLeader;
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
        session = new RemoteSession(pSCPSlave,(SCPSlaveObject*)newSession->GetSession(),pLSCPMaster,m_domainId);
        rsessions[sessionId]=session;
        PrintLog("New session: %d\n",sessionId);
    }
    
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
    
    void SLStateUpdate(com::xvd::neuron::lscp::State *state)
    {
        map<int,RemoteSession*>::iterator it;
        
        it = rsessions.find(state->sessionId);
        if (it != rsessions.end())
        {
            it->second->SLStateUpdate(state);
        }
    }
    
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
    	pSCPSlave = new SCPSlave(pSessionFactoryEventHandler,appId,domainId,"SCP");
        
        // The Controller serves as the admin master for all SFs. 
        pACPSlave = new ACPSlave(pSessionFactoryEventHandler,appId,domainId,"ACP");

        // The Controller serves as the admin master for all SFs. 
        pLSCPMaster = new LSCPMaster(pSessionFactoryEventHandler,appId,domainId,"LSCP");

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
    }

    ~SessionFactory()
    {
        pACstate->state = com::xvd::neuron::OBJECT_STATE_DELETED;        
        pACPSlaveObject->Send(pACstate);
        pACPSlave->DeleteSlaveObject(pACPSlaveObject);
        delete pSCPSlave;
        delete pACPSlave;
        delete pSessionFactoryEventHandler;
    }
    
    void EnterReadyState()
    {
        PrintLog("Entering ready state\n");
        pACstate->state = com::xvd::neuron::OBJECT_STATE_READY;
        pACPSlaveObject->Send(pACstate);
    }
    
    void Shutdown(void)
    {
        PrintLog("Shutting down ....\n");
        doRun = false;
    }

    void HandleCommand(com::xvd::neuron::acp::Control *control)
    {
        PrintLog("Received script {%s}\n",control->script);
    }
    
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
    
    void PrintInfo()
    {
        printf("SF[%04d]: domaindId = %d\n",m_appId,m_domainId);
    }
    
    void PrintLog(const char *logfmt,...)
    {
        va_list vl;
        va_start(vl,logfmt);
        printf("SessionFactory[%d@%d]:",m_appId,m_domainId);
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
    typedef std::map<int,SessionLeader*> SessionLeaderMap;
    
    RemoteSessionMap rsessions;
    SessionLeaderMap SL;
};
