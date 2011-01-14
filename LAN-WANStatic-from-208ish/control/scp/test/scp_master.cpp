#include <stdio.h>
#include <stdlib.h>
#include "ndds_cpp.h"
#include "SCPInterface.h"
#include "SCPMaster.h"
#include "SCPEvent.h"

#include <set>

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
        
        sprintf(control->script,"Created session %d",scp->GetSessionId());        
    }
        
    ~Session()
    {
        // Return the Session object to the SCPMaster
        sm->DeleteMasterObject(scp);
        
        // Delete local memory to store control and state
        com::xvd::neuron::session::ControlTypeSupport::delete_data(control);
    }
    
    // Add a slave to the session. When a slave is added, we send an empty
    // command (create) to trigger a state update from the slave
    void AddSlave(int slaveId)
    {
        slaves.insert(slaveId);
        sprintf(control->script,"create session %d",scp->GetSessionId());
        if (!scp->Send(control,slaveId))
        {
            printf("failed to send create command to slave %d\n",slaveId);
        }
    }

    void RemoveSlave(int slaveId)
    {
        sprintf(control->script,"delete session %d",scp->GetSessionId());
        if (!scp->Send(control,slaveId))
        {
            printf("failed to send remove command to slave %d\n",slaveId);
        }
        else
        {
            slaves.erase(slaveId);
        }
    }
    
    bool SlavesAreReady(set<int> *pending)
    {
        com::xvd::neuron::session::State *state=NULL;
        set<int>::iterator it;        
        for (it = pending->begin(); it != pending->end(); ++it)
        {
            state = scp->GetState(*it);
            if (state == NULL)
            {
                continue;
            }
            if (state->state != com::xvd::neuron::OBJECT_STATE_READY)
            {
                continue;
            }
            pending->erase(*it);
        }
        return pending->empty();
    }
    
    // Example commands
    bool SendCommand(const char *script,int slaveId)
    {
        set<int>::iterator it;
        it = slaves.find(slaveId);
        
        if (it == slaves.end())
        {
            printf("slave %d is not valid\n",slaveId);
            return false;
        }
        
        sprintf(control->script,"execute session %d: %s\n",
                scp->GetSessionId(),script);
        
        return scp->Send(control,slaveId);
    }
    
    void ReportSessionStatus(void)
    {
        printf("Session Status\n");

        com::xvd::neuron::session::State *state=NULL;
        com::xvd::neuron::session::EventSeq *events=NULL;
        com::xvd::neuron::session::MetricsSeq *metrics=NULL;
        
        set<int>::iterator it;
        
        for (it = slaves.begin(); it != slaves.end(); ++it)
        {            
            printf("checking state for %d\n",*it);
            state = scp->GetState(*it);
            events = scp->GetEvents(*it);
            metrics = scp->GetMetrics(*it);
            
            if (state != NULL)
            {
                printf("State: %d, %d,%d\n",state->srcId,state->sessionId,state->state);
            }
            
            if (events != NULL)
            {
                for (int i = 0; i < events->length(); ++i)
                {
                    printf("Event[%d]: %d, %d,%d\n",
                           i,
                           (*events)[i].srcId,
                           (*events)[i].sessionId,
                           (*events)[i].eventCode);
                }
            }
            
            if (metrics != NULL)
            {
                for (int i = 0; i < metrics->length(); ++i)
                {
                    printf("Metrics[%d]: %d, %d,%d,%d,%d\n",
                           i,
                           (*metrics)[i].srcId,
                           (*metrics)[i].sessionId,
                           (*metrics)[i].entityCount,
                           (*metrics)[i].bytesSent,
                           (*metrics)[i].bytesReceived);
                }
            }
        }
    }
        
private:
    SCPMaster *sm;
    SCPMasterObject *scp;
    com::xvd::neuron::session::Control *control;
    std::set<int> slaves;
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

    SCPMasterTester(int appId, int domaindId) : EventHandlerT<SCPMasterTester>()
    { 
    	// Create a SCP Master, appID = 0, doman = 0, Profile = "SCP"
    	sm = new SCPMaster(this,appId,domaindId,"SCP");
    
    	// Create a session with Session ID 25
    	session = new Session(sm,25);
        
        // Same function handles all events
        AddHandleFunc(&SCPMasterTester::MyEventHandler,SCP_EVENT_SESSION_STATE_UPDATE);
        AddHandleFunc(&SCPMasterTester::MyEventHandler,SCP_EVENT_SESSION_METRICS_UPDATE);
        AddHandleFunc(&SCPMasterTester::MyEventHandler,SCP_EVENT_SESSION_EVENT);
    }

    void MyEventHandler(Event *e) 
    {
#if 0
        switch (e->GetKind()) {
            case SCP_EVENT_SESSION_STATE_UPDATE:
                state = (reinterpret_cast<SCPEventSessionStateUpdate*>(e))->GetData();
                printf("State update: sf: %d, session: %d, state=%d\n",state->srcId,state->sessionId,state->state);
                break;
            case SCP_EVENT_SESSION_METRICS_UPDATE:
                metrics = (reinterpret_cast<SCPEventSessionMetricsUpdate*>(e))->GetData();
                printf("Metrics update: sf: %d, session: %d, entityCount=%d,bytesSent=%d,bytesReceived=%d\n",metrics->srcId,metrics->sessionId,metrics->entityCount,metrics->bytesSent,metrics->bytesReceived);
                break;
            case SCP_EVENT_SESSION_EVENT:
                event = (reinterpret_cast<SCPEventSessionEvent*>(e))->GetData();
                printf("Event update: sf: %d, session: %d, eventCode=%d\n",event->srcId,event->sessionId,event->eventCode);
                break;
            default:
                printf("Unknown event: %d\n",e->GetKind());
        }
#endif
    }
	
    void run() 
	{
        int tryCount = 60;
        set<int> pending;
        set<int>::iterator it;        
        
        // This session uses 3 slaves, id=10,11,12
        session->AddSlave(10);
        session->AddSlave(11);
        session->AddSlave(12);
        pending.insert(10);
        pending.insert(11);
        pending.insert(12);

        printf("Waiting for slaves to be ready: ");
        for (it = pending.begin(); it != pending.end(); ++it)
        {
            printf("%d ",*it);
        }
        printf("\n");
        while (!session->SlavesAreReady(&pending) && --tryCount)
        {            
            sleep(1);
            printf("Waiting for slaves to be ready: ");
            for (it = pending.begin(); it != pending.end(); ++it)
            {
                printf("%d ",*it);
                session->AddSlave(*it);
            }
            printf("\n");
        }
        
        if (!tryCount)
        {
            printf("Failed to detect slaves, bailing out\n");
            return;
        }

        session->SendCommand("create srcMANE",10);
        session->SendCommand("create RP",11);
        session->SendCommand("create sinkMANE",12);
        
    	int not_done = 60;
    
    	while (not_done) {
        	sleep(1);
        	not_done--;
        	if (not_done == 55) 
        	{
                session->SendCommand("reduce RP B/W",11);
        	}
        	if (not_done == 50) 
        	{
                session->SendCommand("reduce RP B/W",11);
        	}
        	if (not_done == 45) 
        	{
                session->SendCommand("increase RP B/W",11);
        	}
        	if (not_done == 40) 
        	{
                session->SendCommand("increase RP B/W",11);
        	}
            session->ReportSessionStatus();
            //HandleNextEvent();                
    	}
    
    	delete session;
    	sleep(5);
    } 

    virtual void EventHandleLoop (void)
    {
    }
};

int 
main(int argc, char **argv)
{
    int domainId = 67;
    int appId = 0;
	SCPMasterTester *tester = NULL;
    int i;
    
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
        return -1;
    }
    
    tester = new SCPMasterTester(appId,domainId);
    
    if (tester == NULL)
    {
        return -1;
    }
    
    tester->run();
    
	return 0;
}

