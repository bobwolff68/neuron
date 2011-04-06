//!
//! \file sl.cpp
//!
//! \brief An example session leader
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
//! The purpose of this example is to illustrate how an SL may function
//!
//! Specifically. this code is provided "as is", without warranty, expressed
//! or implied.
//!
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdarg.h>
#include "ndds_cpp.h"
#include "controlplane.h"

class MediaObject {
public:
    virtual void run() = 0;
};

class HelloWorld : public MediaObject {
public:
    HelloWorld(DDSDomainParticipant *participant)
    {
        DDS_TypeCodeFactory *tcFactory;
        DDS_StructMemberSeq hwType;
        DDS_ExceptionCode_t ex;
        DDS_TypeCode *hwTypeCode;
        hwType.maximum(1);
        hwType.length(1);
        DDSTopicDescription *td;
        
        
        td = participant->lookup_topicdescription("HelloWorld");
        
        if ( td != NULL)
        {
            pTopic = DDSTopic::narrow(td);
            return;
        }
        
        hwTypeCode = tcFactory->create_struct_tc("HelloWorld",DDS_StructMemberSeq(),ex);
        hwTypeCode->add_member("msg",
                               -1,
                               tcFactory->create_string_tc(1024,ex),
                               DDS_TYPECODE_NONKEY_MEMBER,
                               ex);
        
        typeSupport = new DDSDynamicDataTypeSupport(hwTypeCode,DDS_DYNAMIC_DATA_TYPE_PROPERTY_DEFAULT);
        typeSupport->register_type(participant, "HelloWorld");
        
        pTopic = participant->create_topic("HelloWorld", 
                                           "HelloWorld",
                                           DDS_TOPIC_QOS_DEFAULT, 
                                           NULL,
                                           DDS_STATUS_MASK_NONE);
        
    }
protected:
    DDSDomainParticipant *pParticipant;
    DDSDynamicDataTypeSupport *typeSupport;
    DDSTopic *pTopic;    
};

//! \class HelloWorldPublisher
//!
//! \brief Example media object publishing HelloWorld
//!
//! This is a sample media object publishing helloworld on DDS Topic. This example
//! uses the DynamicData API to avoid generating type-plugin code
//!
class HelloWorldPublisher : public HelloWorld
{
public:
    HelloWorldPublisher(DDSDomainParticipant *participant,int argc, char **argv) : HelloWorld(participant)
    {
        DDSDataWriter *ddsWriter;
        pPublisher = participant->create_publisher(DDS_PUBLISHER_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);
        
        ddsWriter = pPublisher->create_datawriter(pTopic, 
                                                  DDS_DATAWRITER_QOS_DEFAULT, 
                                                  NULL, 
                                                  DDS_STATUS_MASK_NONE);
        pWriter = DDSDynamicDataWriter::narrow(ddsWriter);
        sample = typeSupport->create_data();
        sampleCount = 0;
        pWriter->enable();
    }
    
    void run()
    {
        sprintf(buffer,"HelloWorld (%d)",sampleCount);
        ++sampleCount;
        sample->set_string("msg", DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED, buffer);
        if (pWriter->write(*sample, DDS_HANDLE_NIL) != DDS_RETCODE_OK)
        {
            printf("failed to write\n");
        }
    }
    
private:
    DDSDynamicDataWriter *pWriter;
    DDSPublisher *pPublisher;
    DDS_DynamicData *sample;
    int sampleCount;
    char buffer[1024];            
};

//! \class HelloWorldSubscriber
//!
//! \brief Example media object subscribing HelloWorld
//!
//! This is a sample media object publishing helloworld on DDS Topic. This example
//! uses the DynamicData API to avoid generating type-plugin code
//!
class HelloWorldSubscriber : public HelloWorld
{
public:
    HelloWorldSubscriber(DDSDomainParticipant *participant,int argc, char **argv) : HelloWorld(participant)
    {
        DDSDataReader *ddsReader;
        pSubscriber = participant->create_subscriber(DDS_SUBSCRIBER_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);
        
        ddsReader = pSubscriber->create_datareader(pTopic, 
                                                  DDS_DATAREADER_QOS_DEFAULT, 
                                                  NULL, 
                                                  DDS_STATUS_MASK_NONE);
        pReader = DDSDynamicDataReader::narrow(ddsReader);
        pReader->enable();
    }
    
    void run()
    {
        DDS_DynamicDataSeq data_seq;
        DDS_SampleInfoSeq info_seq;
        DDS_ReturnCode_t retcode;
        int i;
        
        retcode = pReader->take(data_seq, 
                                info_seq, 
                                DDS_LENGTH_UNLIMITED, 
                                DDS_ANY_SAMPLE_STATE,
                                DDS_ANY_VIEW_STATE, 
                                DDS_ANY_INSTANCE_STATE);
        
        if ((retcode != DDS_RETCODE_NO_DATA) && (retcode != DDS_RETCODE_OK))
        {
            printf("error reading data\n");
            return;
        }
        
        for (i = 0; i < data_seq.length(); ++i)
        {
            if (info_seq[i].valid_data) 
            {
                DDS_Char *msg=NULL;
                data_seq[i].get_string(*&msg,NULL,"msg",DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED);
                if (msg != NULL) 
                {   
                    printf("%s\n",msg);
                } 
            }
        }
        
        pReader->return_loan(data_seq,info_seq);
    }
    
private:
    DDSDynamicDataReader *pReader;
    DDSSubscriber *pSubscriber;
    DDS_DynamicData *sample;
    int sampleCount;
};


//! \class SessionLeader
//! 
//! \brief The SessionLeader encpasulates a single session
//!
//! The SessionLeader class encapsulates all functionality related to
//! managing a single session for session leader. The SessionLeader
//! is typically forked of by a SF and only communicates locally with the SF
//!
class SessionLeader {
public:
    SessionLeader(int appId,int domainId,int sessionId)
    {
        DDSDomainParticipantFactory *factory = DDSDomainParticipantFactory::get_instance();
        m_appId = appId;
        m_domainId = domainId;
        m_sessionId = sessionId;
        
        pLscpState = com::xvd::neuron::lscp::StateTypeSupport::create_data();
        pLscpEvent = com::xvd::neuron::lscp::EventTypeSupport::create_data();
        pLscpMetrics = com::xvd::neuron::lscp::MetricsTypeSupport::create_data();
        
        pSLEventHandler = new SessionLeaderEventHandler(this);

        // The Controller serves as the admin master for all SFs. 
        pLSCPSlave = new LSCPSlave(pSLEventHandler,appId,domainId,"SL::SCPSlave","LSCP");
        
        doRun = true;
        
        pLSCPSlaveObject = pLSCPSlave->CreateSlaveObject(sessionId);
        
        eventThread = RTIOsapiThread_new("SLeventThread", 
                                         RTI_OSAPI_THREAD_PRIORITY_NORMAL, 
                                         0,
                                         PTHREAD_STACK_MIN*4, 
                                         NULL, 
                                         eventLoop, 
                                         this);
        
        // Let the SF know we are ready to receive event. A STANDBY is sent because
        // we want to ensure that the SF has detected the STANDBY state by sending 
        // an enable command
        pLscpState->state = com::xvd::neuron::OBJECT_STATE_STANDBY;
        pLSCPSlaveObject->Send(pLscpState);        
        pDomainParticipant = factory->create_participant(68, 
                                                         DDS_PARTICIPANT_QOS_DEFAULT, 
                                                         NULL, 
                                                         DDS_STATUS_MASK_NONE);
        
        while (doRun)
        {
            usleep(100000);
        }
    }
    
    ~SessionLeader()
    {
        pLscpState->state = com::xvd::neuron::OBJECT_STATE_DELETED;
        pLSCPSlaveObject->Send(pLscpState);
        pLSCPSlave->DeleteSlaveObject(pLSCPSlaveObject);        
        com::xvd::neuron::lscp::StateTypeSupport::delete_data(pLscpState);
        com::xvd::neuron::lscp::EventTypeSupport::delete_data(pLscpEvent);
        com::xvd::neuron::lscp::MetricsTypeSupport::delete_data(pLscpMetrics);
        delete pLSCPSlave;
        delete pSLEventHandler;
    }
    
    //! \brief Handle a new session
    //!
    //! This function handles incoming requests  to create new sessions. However
    //! The SF will only send _one_ session request to any SL. THus, what this
    //! really means is that the SF has detected the STANDBY state, or if we
    //! were already READY, detected that. This completes the handshake between the
    //! SF and SL
    void NewSession(LSCPEventNewSession *newSession)
    {
        
        if (pLscpState->state ==  com::xvd::neuron::OBJECT_STATE_STANDBY)
        {
            com::xvd::neuron::lscp::Control *control;
            
            pLscpState->state = com::xvd::neuron::OBJECT_STATE_READY;
            pLSCPSlaveObject->Send(pLscpState);
            
            control = newSession->GetData();
            
            ExecuteScript(control->script);
        }
        
        // TODO: What to do with the slave object detected? If we delete it it will 
        //       dispose of our keys towards the SF. That in itself is not harmful,
        //       unless the generation counts are used
        //pLSCPSlave->DeleteSlaveObject((LSCPSlaveObject*)newSession->GetSession());
    }
    
    //! \brief Handle updates to the session.
    //!
    //! This function updates the session
    void UpdateSession(com::xvd::neuron::lscp::Control *control)
    {
        if (control->sessionId != m_sessionId)
        {
            PrintLog("Update received for an unknown session!\n");
            return;
        }
    
        pLscpState->state = com::xvd::neuron::OBJECT_STATE_UPDATE;
        pLSCPSlaveObject->Send(pLscpState);
        
        PrintLog("Update session %d: {%s}\n",pLSCPSlaveObject->GetSessionId(),control->script);

        pLscpState->state = com::xvd::neuron::OBJECT_STATE_READY;
        pLSCPSlaveObject->Send(pLscpState);
    }
    
    //! \brief Delete this session
    //!
    //! This function deletes the session    
    void Shutdown()
    {
        PrintLog("Shutting down ....\n");
        doRun = false;
    }
    
    void PrintLog(const char *logfmt,...)
    {
        va_list vl;
        va_start(vl,logfmt);
        printf("SL[%d/%d/%d]:",m_appId,m_domainId,m_sessionId);
        vprintf(logfmt,vl);
        va_end(vl);
    }
    
    //! Only supports pre-defined action
    void ExecuteScript(char *script)
    {
        printf("SL: run script {%s}\n",script);
        HelloWorldPublisher *hwpub = new HelloWorldPublisher(pDomainParticipant,0,NULL);
        HelloWorldSubscriber *hwsub = new HelloWorldSubscriber(pDomainParticipant,0,NULL);
        
        mediaObjects["pub"] = hwpub;
        mediaObjects["sub"] = hwsub;        
    }
    
    void Schedule()
    {
        MediaObjectMap::iterator it;
        for (it = mediaObjects.begin(); it != mediaObjects.end(); ++it)
        {
            it->second->run();
        }
    }
    
private:
    
    static void* eventLoop(void *param)
    {
        SessionLeader *ev = (SessionLeader*)param;
        
        while (ev->doRun)
        {
            usleep(100000);
            ev->pSLEventHandler->HandleNextEvent();
            ev->Schedule();
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
            switch (e->GetKind()) {
                case LSCP_EVENT_NEW_SESSION:
                    sl->NewSession(reinterpret_cast<LSCPEventNewSession*>(e));
                    break;
                case LSCP_EVENT_UPDATE_SESSION:
                    control = reinterpret_cast<LSCPEventUpdateSession*>(e)->GetData();
                    sl->UpdateSession(control);
                    break;
                case LSCP_EVENT_DELETE_SESSION:
                    sl->Shutdown();
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
    int m_sessionId;
    
    //! \var pSCPMaster 
    //! \brief Attachment to the SCP plane
    LSCPSlave *pLSCPSlave;
    
    LSCPSlaveObject *pLSCPSlaveObject;
    
    SessionLeaderEventHandler *pSLEventHandler;
  
    RTIOsapiThread *eventThread;

    com::xvd::neuron::lscp::State *pLscpState;
    com::xvd::neuron::lscp::Event *pLscpEvent;
    com::xvd::neuron::lscp::Metrics *pLscpMetrics;  
    
    typedef map<const char*,MediaObject*> MediaObjectMap;
    MediaObjectMap mediaObjects;
    
    bool doRun;
    
    DDSDomainParticipant *pDomainParticipant;
};

int 
main(int argc, char **argv)
{
    int domainId = 67;
    int appId = 0;
    int sessionId = 0;
    
	SessionLeader *tester = NULL;
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
        else if (!strcmp("-sessionId",argv[i]))
        {
            ++i;
            if (i == argc)
            {
                printf("-sessionId <appId>\n");
                break;
            }
            sessionId = strtol(argv[i],NULL,0);            
        }        
    }        
    
    if (i < argc) {
        printf("SL[%d/%d/%d]: Error kicking off SL\n",appId,domainId,sessionId);
        return -1;
    }
    
    //! This the constructor does not return until the SF terminates
    tester = new SessionLeader(appId,domainId,sessionId);
    
    delete tester;
    
	return 0;
}
