#include <stdio.h>
#include <stdlib.h>
#include "ndds_cpp.h"
#include "SCPInterface.h"
#include "SCPSlave.h"
#include "SCPEvent.h"

// Application Session object.
class Session 
{
public:
    
    // A SCPSlave object detects new session created on the session plane. Thus,
    // it shall never create a session object. Rather, it is received from the
    // SCP.
    Session(SCPSlave *_sl,SCPSlaveObject *_scp)
    {
        scp = _scp;
        sl = _sl;
        control = com::xvd::neuron::session::ControlTypeSupport::create_data();
        state = com::xvd::neuron::session::StateTypeSupport::create_data();
        event = com::xvd::neuron::session::EventTypeSupport::create_data();
        metrics = com::xvd::neuron::session::MetricsTypeSupport::create_data();
        
        sprintf(control->script,"Created session %d",scp->GetSessionId());
        state->state = com::xvd::neuron::OBJECT_STATE_READY;
        scp->Send(state);
    }
    
    ~Session()
    {
        sl->DeleteSlaveObject(scp);
        com::xvd::neuron::session::ControlTypeSupport::delete_data(control);
        com::xvd::neuron::session::StateTypeSupport::delete_data(state);
        com::xvd::neuron::session::EventTypeSupport::delete_data(event);
        com::xvd::neuron::session::MetricsTypeSupport::delete_data(metrics);        
    }
    
    void Update(com::xvd::neuron::session::Control *control)
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
    
private:
    SCPSlave *sl;
    SCPSlaveObject *scp;
    com::xvd::neuron::session::Control *control;
    com::xvd::neuron::session::State *state;
    com::xvd::neuron::session::Event *event;
    com::xvd::neuron::session::Metrics *metrics;
};

class SCPSlaveTester : public EventHandlerT<SCPSlaveTester>
{
public:
    Session *session;
    com::xvd::neuron::session::Control *control;
    com::xvd::neuron::session::State *state;
    com::xvd::neuron::session::Event *event;
    com::xvd::neuron::session::Metrics *metrics;
    SCPSlave *sl;
    
    SCPSlaveTester() : EventHandlerT<SCPSlaveTester>()
    {
        sl = new SCPSlave(this,10,0,"SCP");
        session = NULL;
        AddHandleFunc(&SCPSlaveTester::MyEventHandler,SCP_EVENT_NEW_SESSION);
        AddHandleFunc(&SCPSlaveTester::MyEventHandler,SCP_EVENT_UPDATE_SESSION);
        AddHandleFunc(&SCPSlaveTester::MyEventHandler,SCP_EVENT_DELETE_SESSION);
    }
    
    void MyEventHandler(Event *e) 
    {
        // Check queue for events 
        switch (e->GetKind()) {
            case SCP_EVENT_NEW_SESSION:
            {
                SCPEventNewSession* se = reinterpret_cast<SCPEventNewSession*>(e);
                session = new Session(sl,(SCPSlaveObject*)se->GetSession());
                printf("New session: %d\n",((SCPSlaveObject*)se->GetSession())->GetSessionId());
            }
                break;
            case SCP_EVENT_UPDATE_SESSION:
                control = reinterpret_cast<SCPEventUpdateSession*>(e)->GetData();
                printf("Update session: %d\n",control->sessionId);
                session->Update(control);
                break;
            case SCP_EVENT_DELETE_SESSION:
                printf("Delete session: %d\n",
                       reinterpret_cast<SCPEventDeleteSession*>(e)->GetSessionId());
                delete session;
                session = NULL;
                break;
            default:
                printf("Unknown event: %d\n",e->GetKind());
        }        
    }
    
    void run() {
        while (true) {
            sleep(1);
            if (session != NULL) {
                session->UpdateMetrics();
            }
            HandleNextEvent();
        }
    }
    
    virtual void EventHandleLoop (void)
    {
    }    
};

int 
main(int argc, char **argv)
{
    SCPSlaveTester tester;
	tester.run();
	return 0;
    
}
