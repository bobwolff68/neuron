#include <stdio.h>
#include <stdlib.h>
#include "ndds_cpp.h"
#include "SCPInterface.h"
#include "SCPMaster.h"
#include "SCPEvent.h"

// Application Session object. This class defines what a session is from an 
// application point of view.
class Session 
{
public:
    
    // The constructor takes the SCP master object and session ID as input. The
    // SCP Master attaches to the SCP plane and can manipulate any session. 
    Session(SCPMaster *_sm,int _sid)
    {
        sm = _sm;
        
        // The SCPMasterObject is an instance of a session as seen from the
        // SCPMaster. Each MasterObject maintains state for a single session
        // for this SCPMaster. 
        scp = sm->CreateMasterObject(_sid);
        
        // This data-types defines the interrface to the SCPMasterObject.
        control = com::xvd::neuron::session::ControlTypeSupport::create_data();
        state = com::xvd::neuron::session::StateTypeSupport::create_data();
        event = com::xvd::neuron::session::EventTypeSupport::create_data();
        metrics = com::xvd::neuron::session::MetricsTypeSupport::create_data();
        
        sprintf(control->script,"Created session %d",scp->GetSessionId());
        
        // A session object can be sent to a single destination, in this case
        // THe destination ID is 10
        scp->Send(control,10);
    }
        
    ~Session()
    {
        // Return the Session object to the SCPMaster
        sm->DeleteMasterObject(scp);
        
        // Delete local memory to store control and state
        com::xvd::neuron::session::ControlTypeSupport::delete_data(control);
        com::xvd::neuron::session::StateTypeSupport::delete_data(state);
        com::xvd::neuron::session::EventTypeSupport::delete_data(event);
        com::xvd::neuron::session::MetricsTypeSupport::delete_data(metrics);        
    }
    
    // Example commands
    void AddEntity(const char *script)
    {
        sprintf(control->script,"add script for session(%d): %s\n",
                scp->GetSessionId(),script);
        scp->Send(control,10);
    }

    void RemoveEntity(const char *script)
    {
        sprintf(control->script,"remove script for session(%d): %s\n",
                scp->GetSessionId(),script);
        scp->Send(control,10);
    }
    
    void ReportSessionStatus(void)
    {
        com::xvd::neuron::session::State *state;
        com::xvd::neuron::session::EventSeq *events;
        com::xvd::neuron::session::MetricsSeq *metrics;
        
        state = scp->GetState();
        events = scp->GetEvents();
        metrics = scp->GetMetrics();
    }
        
private:
    SCPMaster *sm;
    SCPMasterObject *scp;
    com::xvd::neuron::session::Control *control;
    com::xvd::neuron::session::State *state;
    com::xvd::neuron::session::Event *event;
    com::xvd::neuron::session::Metrics *metrics;
};

// The SCPMaster EventHandler
class SCPMasterTester : public EventHandlerT<SCPMasterTester>
{
public:
    Session *session;
    com::xvd::neuron::session::State *state;
    com::xvd::neuron::session::Event *event;
    com::xvd::neuron::session::Metrics *metrics;
    SCPMaster *sm;

    SCPMasterTester() : EventHandlerT<SCPMasterTester>()
    { 
    	// Create a SCP Master, appID = 0, doman = 0, Profile = "SCP"
    	sm = new SCPMaster(this,0,0,"SCP");
    
    	// Create a session with Session ID 25
    	session = new Session(sm,25);
        
        // Same function handles all events
        AddHandleFunc(&SCPMasterTester::MyEventHandler,STATE_SESSION_EVENT_ID);
        AddHandleFunc(&SCPMasterTester::MyEventHandler,METRICS_SESSION_EVENT_ID);
        AddHandleFunc(&SCPMasterTester::MyEventHandler,EVENT_SESSION_EVENT_ID);
    }

    void MyEventHandler(Event *e) 
    {
        switch (e->GetKind()) {
            case STATE_SESSION_EVENT_ID:
                state = (reinterpret_cast<SessionStateEvent*>(e))->GetData();
                printf("State update: sf: %d, session: %d, state=%d\n",state->srcId,state->sessionId,state->state);
                break;
            case METRICS_SESSION_EVENT_ID:
                metrics = (reinterpret_cast<SessionMetricsEvent*>(e))->GetData();
                printf("Metrics update: sf: %d, session: %d, entityCount=%d,bytesSent=%d,bytesReceived=%d\n",metrics->srcId,metrics->sessionId,metrics->entityCount,metrics->bytesSent,metrics->bytesReceived);
                break;
            case EVENT_SESSION_EVENT_ID:
                event = (reinterpret_cast<SessionEventEvent*>(e))->GetData();
                printf("Event update: sf: %d, session: %d, eventCode=%d\n",event->srcId,event->sessionId,event->eventCode);
                break;
            default:
                printf("Unknown event: %d\n",e->GetKind());
        }
    }
	
    void run() 
	{

    	sleep(10);
    
    	int not_done = 60;
    
    	while (not_done) {
        	sleep(1);
        	not_done--;
        	if (not_done == 55) 
        	{
            		session->AddEntity("{e0,e1,e2 ....}\n");
        	}
        	if (not_done == 50) 
        	{
            		session->AddEntity("{e4,e5,e6 ....}\n");
        	}
        	if (not_done == 30) 
        	{
            		session->RemoveEntity("{e0,e1,e2,e3 ....}\n");
        	}
        	if (not_done == 20) 
        	{
            		session->RemoveEntity("{e5,e6 ....}\n");
        	}
        	if (not_done % 10) {
            		session->ReportSessionStatus();
       		} 
            HandleNextEvent();
    	}
    
    	delete session;
    } 

    virtual void EventHandleLoop (void)
    {
    }
};

int 
main(int argc, char **argv)
{
	SCPMasterTester tester;
	tester.run();
	return 0;
}

