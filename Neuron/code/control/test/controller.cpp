#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "ndds_cpp.h"
#include "neuroncommon.h"
#include "controlplane.h"

#include <set>
#include <map>
#include <list>

#include "ep.cpp"
#include "sl.cpp"
#include "sf.cpp"

void CreateArgvList(char *string,int *argc,char **argv)
{
    char *c = string;
    int s = 0;
    
    *argc = 0;
    while (1)
    {
        if (!*c)
        {
            break;
        }
        switch(s)
        {
            case 0:
                if (!isblank(*c))
                {
                    if (*c == '"')
                        s = 2;
                    else
                        s = 1;
                    argv[(*argc)++] = c;
                }
                break;
            case 1:
                if (isblank(*c))
                {
                    *c = 0;
                    s = 0;
                }             
            case 2:
                if (*c == '"')
                {
                    ++c;
                    *c = 0;
                    s = 0;
                }
                break;
        }
        ++c;
    }
}

class Controller
{
    
public:
    Controller(int appId, int domaindId)
    { 
        pSessionEventHandler = new ControllerEventHandler(this);
                
        // The Controller manages sessions
    	pSCPMaster = new SCPMaster(pSessionEventHandler,appId,domaindId,"SCP");

        // The Controller serves as the admin master for all SFs. 
        pACPMaster = new ACPMaster(pSessionEventHandler,appId,domaindId,"ACP");
        
        m_domainId = domaindId;
        
        eventThread = RTIOsapiThread_new("controlThread", 
                                         RTI_OSAPI_THREAD_PRIORITY_NORMAL, 
                                         0, 
                                         PTHREAD_STACK_MIN*16, 
                                         NULL, 
                                         eventLoop, 
                                         pSessionEventHandler);
    }
    
    
    ~Controller()
    {
        pACPMaster->DeleteMasterObject(pACPMasterObject);
        delete pSCPMaster;
        delete pACPMaster;
        delete pSessionEventHandler;
    }

    void RemoteSessionUpdate(com::xvd::neuron::scp::State *state)
    {
        printf("State update: sf: %d, scp: %d, state=%d\n",state->srcId,state->sessionId,state->state);
    }
    
    void RemoteSFUpdate(com::xvd::neuron::acp::State *state)
    {
        RemoteSessionFactoryMap::iterator it;
        RemoteSessionFactory *rsf;
        
        it = remoteSF.find(state->srcId);
        if (it == remoteSF.end())
        {
            ACPMasterObject *pACPMasterObject;
            printf("New SF detected\n");
            pACPMasterObject = pACPMaster->CreateMasterObject(state->srcId);
            rsf = new RemoteSessionFactory(state->srcId,pACPMasterObject);
            remoteSF[state->srcId] = rsf;
        }
        else
        {
            printf("Update to SF detected, state = %d\n",state->state);
            if (state->state == com::xvd::neuron::OBJECT_STATE_DELETED)
            {
                RemoteSessionFactoryMap::iterator rsfit;
                SessionFactory *sf;
                rsfit = remoteSF.find(state->srcId);
                if (rsfit != remoteSF.end())
                {
                    //pACPMaster->DeleteMasterObject(rsfit->second->GetSCPMasterObject());
                    //delete rsfit->second;
                    remoteSF.erase(state->srcId);
                    //sf = SFList[sId]
                    //(*argc)++;
                }
                else 
                {
                    printf("SessionFactory %d does not exist\n",state->srcId);
                }                
            }
            
        }
    }
    
    void CmdHelp()
    {
        printf("Available commands:\n\n");        
        printf("acp ls\n");
        printf("acp shutdown <id>\n");
        printf("acp send <sfId>+ <script>\n");
        printf("scp create <sessionId> <id>+ <script>\n");
        printf("scp send <sessionId> <id>+ <script>\n");
        printf("scp delete <sessionId>\n");
        printf("scp ls\n");
        printf("sf create <id>\n");
        printf("sf delete <id>\n");
        printf("sf ls\n");
        printf("\n");        
        printf("quit\n");
        printf("\n");
    }
    
    void CmdACP(int *argc,int max_argc,char **argv)
    {
        ++(*argc);
        if (*argc == max_argc)
        {
            printf("usage: acp <cmd>\n");
            return;
        }
        if (!strcmp(argv[*argc],"ls"))
        {
            ListRemoteSF();
        }
        else if (!strcmp(argv[*argc],"shutdown"))
        {
            int sfId;
            ++(*argc);
            if (*argc == max_argc)
            {
                printf("usage: acp shutdown <id>\n");
                return;
            }
            sfId = strtol(argv[*argc],NULL,0);
            ShutdownRemoteSF(sfId);
        }                                
        else if (!strcmp(argv[*argc],"send"))
        {
            int sfId;
            set<int> ids;
            set<int>::iterator it;
            RemoteSessionFactoryMap::iterator rsfit;
            
            ++(*argc);
            if (*argc == max_argc)
            {
                printf("usage: acp send <id>+ <cmd>\n");
                return;
            }            
            while (*argc < max_argc)
            {
                if (argv[*argc][0]=='"')
                    break;
                sfId = strtol(argv[*argc],NULL,0);
                ids.insert(sfId);
                (*argc)++;
            }
            if (*argc == max_argc-1)
            {
                for (it = ids.begin(); it != ids.end(); ++it)
                {
                    rsfit = remoteSF.find(*it);
                    rsfit->second->Send(argv[*argc]);
                }
            }
            else 
            {
                printf("usage: acp send <id>+ <cmd>\n");
                return;
            }
        }                                        
        else
        {
            printf("unknown acp command: %s\n",argv[*argc]);
        }        
    }
    
    void CmdSCP(int *argc,int max_argc,char **argv)
    {
        ++(*argc);
        if (*argc == max_argc)
        {
            printf("usage: scp <cmd>\n");
            return;
        }
        if (!strcmp(argv[*argc],"create") || !strcmp(argv[*argc],"update"))
        {
            int sId;
            int sfId;
            set<int> sessionSFs;
            bool update = !strcmp(argv[*argc],"update");
            
            ++(*argc);
            if (*argc == max_argc)
            {
                printf("usage: scp [create | update] <sessionId>+ <cmd>\n");
                return;
            }
            sId = strtol(argv[*argc],NULL,0);
            
            ++(*argc);
            while (*argc < max_argc)
            {
                if (argv[*argc][0]=='"')
                    break;
                sfId = strtol(argv[*argc],NULL,0);
                sessionSFs.insert(sfId);
                (*argc)++;
            }
            if (*argc == max_argc-1)
            {
                CreateOrUpdateSession(sId,&sessionSFs,argv[*argc],update);
            }
            else 
            {
                CreateOrUpdateSession(sId,&sessionSFs,NULL,update);
            }
        }
        else if (!strcmp(argv[*argc],"delete"))
        {
            int sId;
            ++(*argc);
            if (*argc == max_argc)
            {
                printf("usage: scp delete <sessionId>\n");
                return;
            }
            sId = strtol(argv[*argc],NULL,0);
            DeleteSession(sId);
        }                    
        else if (!strcmp(argv[*argc],"ls"))
        {
            ListSession();
        }
        else
        {
            printf("unknown scp command: %s\n",argv[*argc]);
        }

    }
    
    void CmdSF(int *argc,int max_argc,char **argv)
    {
        int sId;
        SessionFactory *aSF;
        
        ++(*argc);
        if (*argc == max_argc)
        {
            printf("usage: sf <cmd>\n");
            return;
        }
        if (!strcmp(argv[*argc],"create"))
        {
            ++(*argc);
            if (*argc == max_argc)
            {
                printf("usage: sf create <id>\n");                
                return;
            }
            while (*argc < max_argc)
            {
                sId = strtol(argv[*argc],NULL,0);
                aSF = new SessionFactory(sId,m_domainId);
                SFList[sId] = aSF;                        
                (*argc)++;
            }            
        }
        else if (!strcmp(argv[*argc],"ls"))
        {
            ListSF();
        }
        else if (!strcmp(argv[*argc],"delete"))
        {
            std::map<int,SessionFactory*>::iterator it;
            ++(*argc);
            if (*argc == max_argc)
            {
                printf("usage: sf delete <id>\n");                
                return;
            }                        
            sId = strtol(argv[*argc],NULL,0);
            it = SFList.find(sId);
            delete it->second;
            SFList.erase(sId);
        }                    
    }
    
    void ListRemoteSF()
    {
        RemoteSessionFactoryMap::iterator it;
        
        printf("\nRemote SessionFactories\n");
        printf("=======================\n");
        for (it = remoteSF.begin(); it != remoteSF.end(); ++it)
        {
            it->second->PrintInfo();
        }
        printf("\n");
    }

    void ListSF()
    {
        map<int,SessionFactory*>::iterator it;
        
        printf("\nLocal SessionFactories\n");
        printf("======================\n");
        for (it = SFList.begin(); it != SFList.end(); ++it)
        {
            it->second->PrintInfo();
        }
        printf("\n");
    }

    void ListSL()
    {
        map<int,SessionLeader*>::iterator it;
        
        printf("SessionLeaders:\n");
        for (it = SL.begin(); it != SL.end(); ++it)
        {
            it->second->PrintInfo();
        }        
    }
    
    void ListMANE()
    {
        std::map<int,MANE*>::iterator it;
        
        printf("MANE's:\n");
        for (it = MANEList.begin(); it != MANEList.end(); ++it)
        {
            it->second->PrintInfo();
        }        
    }
    
    void ShutdownRemoteSF(int sfId)
    {
        printf("Shutting down SessionFactory %d ....\n",sfId);
        RemoteSessionFactoryMap::iterator rsfit;
        
        rsfit = remoteSF.find(sfId);
        if (rsfit != remoteSF.end())
        {
            pACPMaster->DeleteMasterObject(rsfit->second->GetSCPMasterObject());
            //delete rsfit->second;
            //remoteSF.erase(sfId);
        }
        else 
        {
            printf("SessionFactory %d does not exist\n",sfId);
        }
    }
    
    void CreateOrUpdateSession(int sessionId, set<int> *_sfs,const char *script,bool update)
    {
        set<int>::iterator it;
        RemoteSessionFactoryMap::iterator rsfit;
        SessionMap::iterator sit;
        Session *session;
        
        sit = sessions.find(sessionId);
        if (sit != sessions.end())
        {
            printf("Session %d already exists\n",sessionId);
            return;
        }
        
        if (!update)
        {
            printf("Create session %d using SF ",sessionId);
        }
        else 
        {
            printf("Update session %d using SF ",sessionId);
        }
        session = new Session(pSCPMaster,sessionId);
        for (it = _sfs->begin(); it != _sfs->end(); ++it)
        {
            rsfit = remoteSF.find(*it);
            if (rsfit == remoteSF.end())
            {
                printf("ERROR: SF id does not exist\n");
                return;
            }
            if (!rsfit->second->IsAvailable()) 
            {
                printf("ERROR: SF %d is not available\n",rsfit->first);
                return;
            }
            session->AddSlave(*it);
            printf("%d ",*it);
        }
        if (script != NULL)
        {
            printf("with script %s\n",script);
        }
        else
        {
            printf("\n");
        }
        sessions[sessionId] = session;
    }
    
    void ListSession()
    {
        SessionMap::iterator it;
        
        printf("\nSessions\n");
        printf("========\n");
        
        for (it = sessions.begin(); it != sessions.end(); ++it)
        {
            it->second->ReportSessionStatus();
        }        
        printf("\n");
    }
    
    void DeleteSession(int sId)
    {
        SessionMap::iterator it;

        it = sessions.find(sId);
        if (it != sessions.end())
        {
            printf("Delete session %d\n",sId);
            delete it->second;
            sessions.erase(sId);
        } 
        else
        {
            printf("Session %d does not exist\n",sId);
        }
    }
    
    void run()
    {
        int quit = 0;
        char *line;
        char **argv = (char**)calloc(sizeof(char*),200);
        int argc;
        int s;
        
        using_history();
        
        while (!quit)
        {
            line = readline("controller>");
            if (!strcmp(line,"quit")) 
            {
                quit = 1;
                continue;
                
            }

            add_history(line);
            
            CreateArgvList(strdup(line),&argc,argv);

            for (s = 0; s < argc; ++s)
            {
                if (!strcmp(argv[s],"help"))
                {
                    CmdHelp();
                    break;
                }
                else if (!strcmp(argv[s],"acp"))
                {
                    CmdACP(&s,argc,argv);
                }
                else if (!strcmp(argv[s],"scp"))
                {
                    CmdSCP(&s,argc,argv);
                }   
                else if (!strcmp(argv[s],"sf"))
                {
                    CmdSF(&s,argc,argv);
                }
                else 
                {
                    printf("Unknown command: %s\n",argv[s]);
                    break;
                }
            }
        }
    }
        
private:

    class Session 
    {
    public:
        
        // The constructor takes the SCP master object and scp ID as input. The
        // SCP Master attaches to the SCP plane and can manipulate any scp. 
        Session(SCPMaster *_sm,int _sid)
        {
            sm = _sm;
            
            // The SCPMasterObject is an instance of a session as seen from the
            // SCPMaster. Each MasterObject maintains state for a single session
            // forthis SCPMaster. 
            scp = sm->CreateMasterObject(_sid);
            
            // Data-type for the interface to the SCPMasterObject.
            control = com::xvd::neuron::scp::ControlTypeSupport::create_data();
            
            sprintf(control->script,"Created scp %d",scp->GetSessionId());        
        }
        
        ~Session()
        {
            // Return the session object to the SCPMaster
            if (!sm->DeleteMasterObject(scp))
            {
                printf("error deleting master object\n");
            }
            
            // Delete local memory to store control and state
            com::xvd::neuron::scp::ControlTypeSupport::delete_data(control);
        }
        
        // Add a slave to the session. When a slave is added, we send an empty
        // command (create) to trigger a state update from the slave
        void AddSlave(int slaveId)
        {
            sfs.insert(slaveId);
            sprintf(control->script,"create session %d",scp->GetSessionId());
            if (!scp->Send(control,slaveId))
            {
                printf("failed to send create command to slave %d\n",slaveId);
            }
        }
        
        void RemoveSlave(int slaveId)
        {
            sprintf(control->script,"delete scp %d",scp->GetSessionId());
            if (!scp->Send(control,slaveId))
            {
                printf("failed to send remove command to slave %d\n",slaveId);
            }
            else
            {
                sfs.erase(slaveId);
            }
        }
        
        bool SlavesAreReady(set<int> *pending)
        {
            com::xvd::neuron::scp::State *state=NULL;
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
        
        // Send a command to a SF
        bool SendCommand(const char *script,int slaveId)
        {
            set<int>::iterator it;
            it = sfs.find(slaveId);
            
            if (it == sfs.end())
            {
                printf("slave %d is not valid\n",slaveId);
                return false;
            }
            
            sprintf(control->script,"execute scp %d: %s\n",
                    scp->GetSessionId(),script);
            
            return scp->Send(control,slaveId);
        }
        
        void ReportSessionStatus(void)
        {
            printf("Session [%d] status\n",scp->GetSessionId());
            
            com::xvd::neuron::scp::State *state=NULL;
            com::xvd::neuron::scp::EventSeq *events=NULL;
            com::xvd::neuron::scp::MetricsSeq *metrics=NULL;
            
            set<int>::iterator it;
            
            for (it = sfs.begin(); it != sfs.end(); ++it)
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
        com::xvd::neuron::scp::Control *control;
        std::set<int> sfs;
    };
    
    class RemoteSessionFactory
    {
    public:
        RemoteSessionFactory(int appId,ACPMasterObject *p)
        {
            m_appId = appId;
            pACPMasterObject = p;
            control = com::xvd::neuron::acp::ControlTypeSupport::create_data();
            // As soon as we detect a new SF, start it
            sprintf(control->script,"start");
            pACPMasterObject->Send(control,appId);
        }
        
        ~RemoteSessionFactory()
        {
            com::xvd::neuron::acp::ControlTypeSupport::delete_data(control);
        }
        
        void PrintInfo()
        {
            com::xvd::neuron::acp::State *pACstate;
            com::xvd::neuron::acp::EventSeq *pACeventSeq;
            com::xvd::neuron::acp::MetricsSeq *pACmetricsSeq;
            
            pACstate = pACPMasterObject->GetState(m_appId);
            pACeventSeq = pACPMasterObject->GetEvents(m_appId);
            pACmetricsSeq = pACPMasterObject->GetMetrics(m_appId);
            
            printf("SF[%d]\n",m_appId);
            if (pACstate != NULL)
            {
                printf("\tState=%d\n",pACstate->state);
            }
            else 
            {
                printf("\tState=Unknown\n");
            }
            
            if (pACeventSeq != NULL)
            {
                for (int i = 0; i < pACeventSeq->length(); ++i)
                {
                    printf("\tEvent[%d]: %d, %d\n",
                           i,
                           (*pACeventSeq)[i].srcId,
                           (*pACeventSeq)[i].eventCode);
                }
            }
            
            if (pACmetricsSeq != NULL)
            {
                for (int i = 0; i < pACmetricsSeq->length(); ++i)
                {
                    printf("\tMetrics[%d]: %d, %d,%d,%d\n",
                           i,
                           (*pACmetricsSeq)[i].srcId,
                           (*pACmetricsSeq)[i].entityCount,
                           (*pACmetricsSeq)[i].bytesSent,
                           (*pACmetricsSeq)[i].bytesReceived);
                }
            }
            printf("\n");
        }
        
        bool IsAvailable()
        {
            com::xvd::neuron::acp::State *pACstate;
            com::xvd::neuron::acp::EventSeq *pACeventSeq;
            com::xvd::neuron::acp::MetricsSeq *pACmetricsSeq;
            
            pACstate = pACPMasterObject->GetState(m_appId);
            pACeventSeq = pACPMasterObject->GetEvents(m_appId);
            pACmetricsSeq = pACPMasterObject->GetMetrics(m_appId);
            
            if ((pACstate == NULL) || (pACstate->state != com::xvd::neuron::OBJECT_STATE_READY))
                return false;
            
            // TODO: Add some logic related to metrics
            
            return true;
        }
        
        ACPMasterObject *GetSCPMasterObject()
        {
            return pACPMasterObject;
        }
        
        void Send(char *cmd)
        {
            sprintf(control->script,"%s",cmd);
            pACPMasterObject->Send(control,m_appId);
        }
        
    private:
        int m_appId;
        
        ACPMasterObject *pACPMasterObject;
        com::xvd::neuron::acp::Control *control;
    };
    

    class ControllerEventHandler : public EventHandlerT<ControllerEventHandler>
    {
    public:
        com::xvd::neuron::scp::State *state;
        com::xvd::neuron::scp::Event *event;
        com::xvd::neuron::scp::Metrics *metrics;
        com::xvd::neuron::acp::State *pACstate;
        com::xvd::neuron::acp::Event *pACevent;
        com::xvd::neuron::acp::Metrics *pACetrics;        
        Controller *controller;
        
        ControllerEventHandler(Controller *_controller) : EventHandlerT<ControllerEventHandler>()
        { 
            AddHandleFunc(&ControllerEventHandler::MyEventHandler,SCP_EVENT_SESSION_STATE_UPDATE);
            AddHandleFunc(&ControllerEventHandler::MyEventHandler,SCP_EVENT_SESSION_METRICS_UPDATE);
            AddHandleFunc(&ControllerEventHandler::MyEventHandler,SCP_EVENT_SESSION_EVENT);
            AddHandleFunc(&ControllerEventHandler::MyEventHandler,ACP_EVENT_SESSION_STATE_UPDATE);
            AddHandleFunc(&ControllerEventHandler::MyEventHandler,ACP_EVENT_SESSION_METRICS_UPDATE);
            AddHandleFunc(&ControllerEventHandler::MyEventHandler,ACP_EVENT_SESSION_EVENT);  
            controller = _controller;
        }
        
        void MyEventHandler(Event *e) 
        {
            switch (e->GetKind()) {
                case SCP_EVENT_SESSION_STATE_UPDATE:
                    state = (reinterpret_cast<SCPEventSessionStateUpdate*>(e))->GetData();
                    controller->RemoteSessionUpdate(state);
                    break;
                case SCP_EVENT_SESSION_METRICS_UPDATE:
                    metrics = (reinterpret_cast<SCPEventSessionMetricsUpdate*>(e))->GetData();
                    printf("Metrics update: sf: %d, scp: %d, entityCount=%d,bytesSent=%d,bytesReceived=%d\n",metrics->srcId,metrics->sessionId,metrics->entityCount,metrics->bytesSent,metrics->bytesReceived);
                    break;
                case SCP_EVENT_SESSION_EVENT:
                    event = (reinterpret_cast<SCPEventSessionEvent*>(e))->GetData();
                    printf("Event update: sf: %d, scp: %d, eventCode=%d\n",event->srcId,event->sessionId,event->eventCode);
                    break;
                case ACP_EVENT_SESSION_STATE_UPDATE:
                    pACstate = (reinterpret_cast<ACPEventSessionStateUpdate*>(e))->GetData();
                    controller->RemoteSFUpdate(pACstate);
                    break;
                case ACP_EVENT_SESSION_METRICS_UPDATE:
                    printf("Received ACP metrics update\n");
                    break;
                case ACP_EVENT_SESSION_EVENT:
                    printf("Received ACP event update\n");
                    break;                    
                default:
                    printf("Unknown event: %d\n",e->GetKind());
            }
        }
        
        virtual void EventHandleLoop (void)
        {
        }    
    };

    static void* eventLoop(void *param)
    {
        ControllerEventHandler *ev = (ControllerEventHandler*)param;
        while (true)
        {
            usleep(100000);
            ev->HandleNextEvent();
        }        
        return NULL;
    }
    
    typedef std::map<int,RemoteSessionFactory *> RemoteSessionFactoryMap;
    typedef std::map<int,Session *> SessionMap;

    //! \var pSCPMaster 
    //! \brief Attachment to the SCP plane
    std::map<int,class SessionFactory*> SFList;
    
    //! \var pSCPMaster 
    //! \brief Attachment to the SCP plane
    std::map<int,SessionLeader*> SL;
    
    //! \var pSCPMaster 
    //! \brief Attachment to the SCP plane
    std::map<int,MANE*> MANEList;
    
    //! \var pSCPMaster 
    //! \brief Attachment to the SCP plane
    SCPMaster *pSCPMaster;
    
    //! \var pACPMaster 
    //! \brief Attachment to the ACP plane
    ACPMaster *pACPMaster;
    
    //! \var pACPMasterObject
    //! \brief Objects used to manage SFs. Only one is needed since there is 
    //!        only a single management "session"
    ACPMasterObject *pACPMasterObject;
    
    //! \var sfs
    //! \brief Track all SessionFactories. Key is the ID of the SF
    RemoteSessionFactoryMap remoteSF;

    //! \var sessions
    //! \brief Track all Sessions. Key is the ID of the session
    SessionMap sessions;
    
    ControllerEventHandler *pSessionEventHandler;
    
    int m_domainId;

    RTIOsapiThread *eventThread;
};

int 
main(int argc, char **argv)
{
    int domainId = 67;
    int appId = 0;
	Controller *tester = NULL;
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
        return -1;
    }
    
    tester = new Controller(appId,domainId);
    
    if (tester == NULL)
    {
        return -1;
    }
    
    tester->run();
    
	return 0;
}

