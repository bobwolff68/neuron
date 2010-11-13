#include <stdio.h>
#include <stdlib.h>
#include "ndds_cpp.h"
#include "controlplane.h"

class SessionLeader {
public:
    SessionLeader(int appId,int domainId,int sessionId)
    {
        m_appId = appId;
        m_domainId = domainId;

        pLscpState = com::xvd::neuron::lscp::StateTypeSupport::create_data();
        pLscpEvent = com::xvd::neuron::lscp::EventTypeSupport::create_data();
        pLscpMetrics = com::xvd::neuron::lscp::MetricsTypeSupport::create_data();
        
        pSLEventHandler = new SessionLeaderEventHandler(this);

        // The Controller serves as the admin master for all SFs. 
        pLSCPSlave = new LSCPSlave(pSLEventHandler,appId,domainId,"LSCP");
        
        // The Controller serves as the admin master for all SFs. 
        pECPMaster = new ECPMaster(pSLEventHandler,appId,domainId,"ECP");
        
        doRun = true;
        
        eventThread = RTIOsapiThread_new("SLeventThread", 
                                         RTI_OSAPI_THREAD_PRIORITY_NORMAL, 
                                         0,
                                         PTHREAD_STACK_MIN*4, 
                                         NULL, 
                                         eventLoop, 
                                         this);
    }
    
    ~SessionLeader()
    {
        RTIOsapiThread_delete(eventThread);
        pLscpState->state = com::xvd::neuron::OBJECT_STATE_DELETE;
        pLSCPSlaveObject->Send(pLscpState);
        pLscpState->state = com::xvd::neuron::OBJECT_STATE_DELETED;
        pLSCPSlaveObject->Send(pLscpState);
        pLSCPSlave->DeleteSlaveObject(pLSCPSlaveObject);        
        com::xvd::neuron::lscp::StateTypeSupport::delete_data(pLscpState);
        com::xvd::neuron::lscp::EventTypeSupport::delete_data(pLscpEvent);
        com::xvd::neuron::lscp::MetricsTypeSupport::delete_data(pLscpMetrics);
        delete pLSCPSlave;
        delete pECPMaster;
        delete pSLEventHandler;
    }
    
    void PrintInfo()
    {
        PrintLog("");
    }
    
    void NewSession(LSCPEventNewSession *newSession)
    {
        com::xvd::neuron::lscp::Control *control;
        if (pLSCPSlaveObject != NULL)
        {
            PrintLog("Already handling a session, bailing out\n");
            pLSCPSlave->DeleteSlaveObject((LSCPSlaveObject*)newSession->GetSession());
            return;
        }
        pLSCPSlaveObject = (LSCPSlaveObject*)newSession->GetSession();
        control = newSession->GetData();
        
        PrintLog("New session %d={%s}\n",pLSCPSlaveObject->GetSessionId(),control->script);

        pLscpState->state = com::xvd::neuron::OBJECT_STATE_READY;
        pLSCPSlaveObject->Send(pLscpState);
    }
    
    void UpdateSession(com::xvd::neuron::lscp::Control *control)
    {
        if (pLSCPSlaveObject == NULL)
        {
            PrintLog("pLSCPSlaveObject == NULL, bailing out\n");
        }
        
        pLscpState->state = com::xvd::neuron::OBJECT_STATE_UPDATE;
        pLSCPSlaveObject->Send(pLscpState);
        
        PrintLog("Update session %d\n",pLSCPSlaveObject->GetSessionId());

        pLscpState->state = com::xvd::neuron::OBJECT_STATE_READY;
        pLSCPSlaveObject->Send(pLscpState);
    }
    
    void Shutdown()
    {
        PrintLog("Shutting down ....\n");
        doRun = false;
        
    }
    
    void PrintLog(const char *logfmt,...)
    {
        va_list vl;
        va_start(vl,logfmt);
        printf("SessionLeader[%d@%d]:",m_appId,m_domainId);
        vprintf(logfmt,vl);
        va_end(vl);
    }
    
private:
    class RemoteMANE 
    {
    public:
        
        // A SCPSlave object detects new session created on the session plane. Thus,
        // it shall never create a session object. Rather, it is received from the
        // SCP.
        RemoteMANE(LSCPSlave *_sl,LSCPSlaveObject *_scp)
        {
#if 0
            scp = _scp;
            sl = _sl;
            control = com::xvd::neuron::lscp::ControlTypeSupport::create_data();
            state = com::xvd::neuron::lscp::StateTypeSupport::create_data();
            event = com::xvd::neuron::lscp::EventTypeSupport::create_data();
            metrics = com::xvd::neuron::lscp::MetricsTypeSupport::create_data();
            
            sprintf(control->script,"Created session %d",scp->GetSessionId());
            state->state = com::xvd::neuron::OBJECT_STATE_READY;
            scp->Send(state);
#endif
        }
        
        RemoteMANE()
        {
#if 0
            sl->DeleteSlaveObject(scp);
            com::xvd::neuron::lscp::ControlTypeSupport::delete_data(control);
            com::xvd::neuron::lscp::StateTypeSupport::delete_data(state);
            com::xvd::neuron::lscp::EventTypeSupport::delete_data(event);
            com::xvd::neuron::lscp::MetricsTypeSupport::delete_data(metrics);        
#endif
        }
        
        void Update(com::xvd::neuron::lscp::Control *control)
        {
        }
        
        void UpdateMetrics(void)
        {
        }
        
        
    private:
        ECPSlave *sl;
        ECPSlaveObject *scp;
        com::xvd::neuron::ecp::Control *control;
        com::xvd::neuron::ecp::State *state;
        com::xvd::neuron::ecp::Event *event;
        com::xvd::neuron::ecp::Metrics *metrics;
    };
    
    static void* eventLoop(void *param)
    {
        SessionLeader *ev = (SessionLeader*)param;
        
        while (ev->doRun)
        {
            usleep(100000);
            ev->pSLEventHandler->HandleNextEvent();
        }
        delete ev;
        return NULL;
    }
    
    class SessionLeaderEventHandler : public EventHandlerT<SessionLeaderEventHandler>
    {
    public:
        com::xvd::neuron::lscp::Control *control;
        
        SessionLeader *sl;
        
        SessionLeaderEventHandler(SessionLeader *_sl) : EventHandlerT<SessionLeaderEventHandler>()
        { 
            sl = _sl;
            AddHandleFunc(&SessionLeaderEventHandler::MyEventHandler,LSCP_EVENT_NEW_SESSION);
            AddHandleFunc(&SessionLeaderEventHandler::MyEventHandler,LSCP_EVENT_UPDATE_SESSION);
            AddHandleFunc(&SessionLeaderEventHandler::MyEventHandler,LSCP_EVENT_DELETE_SESSION);
        }
        
        void MyEventHandler(Event *e) 
        {
            com::xvd::neuron::lscp::Control *control;

            switch (e->GetKind()) {
                case LSCP_EVENT_NEW_SESSION:
                {
                    LSCPEventNewSession* se = reinterpret_cast<LSCPEventNewSession*>(e);
                    sl->NewSession(se);
                }
                    break;
                case LSCP_EVENT_UPDATE_SESSION:
                    control = reinterpret_cast<LSCPEventUpdateSession*>(e)->GetData();
                    sl->UpdateSession(control);
                    break;
                case LSCP_EVENT_DELETE_SESSION:
                    sl->Shutdown();
                    //sl->DeleteSession(reinterpret_cast<LSCPEventDeleteSession*>(e)->GetSessionId());
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
    
    int m_appId;
    int m_domainId;
    
    //! \var pSCPMaster 
    //! \brief Attachment to the SCP plane
    LSCPSlave *pLSCPSlave;
    
    LSCPSlaveObject *pLSCPSlaveObject;
    
    //! \var pLSCPMaster 
    //! \brief Attachment to the LSCP plane
    ECPMaster *pECPMaster;
    
    SessionLeaderEventHandler *pSLEventHandler;
  
    RTIOsapiThread *eventThread;

    com::xvd::neuron::lscp::State *pLscpState;
    com::xvd::neuron::lscp::Event *pLscpEvent;
    com::xvd::neuron::lscp::Metrics *pLscpMetrics;  
    
    bool doRun;
    
};
