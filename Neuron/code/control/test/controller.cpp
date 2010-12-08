CMakeLists.txt                                                                                      0000755 0001750 0001750 00000003300 11475364416 013464  0                                                                                                    ustar   manjesh                         manjesh                                                                                                                                                                                                                ##############

PROJECT( test ) # if you want to

cmake_minimum_required(VERSION 2.4)

###################
###################

set(SOURCES_CONTROLLER controller.cpp)
set(SOURCES_SF sf.cpp)
set(SOURCES_SL sl.cpp)

###################
###################

SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY
        bin
        CACHE PATH
        "Single output for all executables"
        )

INCLUDE_DIRECTORIES( ../../neuroncommon ../include ../../include .. ../idl)
INCLUDE_DIRECTORIES( $ENV{NDDSHOME}/include $ENV{NDDSHOME}/include/ndds )

foreach(cp scp ecp lscp acp)
INCLUDE_DIRECTORIES(../${cp}/idl)
INCLUDE_DIRECTORIES(../${cp}/)
endforeach()

add_definitions(-DRTI_LINUX)
add_definitions(-DRTI_UNIX)
add_definitions(-g3)

message(STATUS "cmake_system_processor is: ${CMAKE_SYSTEM_PROCESSOR}")

source_group(Qos USER_QOS_PROFILES.xml)

find_library(PT_LIB pthread)
find_library(NDDSCPP libnddscppz.a $ENV{NDDSHOME}/lib/i86Linux2.6gcc4.1.1)
find_library(NDDSC libnddscz.a $ENV{NDDSHOME}/lib/i86Linux2.6gcc4.1.1)
find_library(NDDSCORE libnddscorez.a $ENV{NDDSHOME}/lib/i86Linux2.6gcc4.1.1)
find_library(COMMON libneuroncommon.a ../../lib)
find_library(CONTROL libcontrolplane.a  ../../lib)
find_library(READLINE readline)

ADD_CUSTOM_TARGET(Resources SOURCES USER_QOS_PROFILES.xml)
ADD_EXECUTABLE(controller ${SOURCES_CONTROLLER})
TARGET_LINK_LIBRARIES(controller ${CONTROL} ${COMMON} ${NDDSCPP} ${NDDSC} ${NDDSCORE} ${PT_LIB} ${READLINE} -ldl -lrt)

ADD_EXECUTABLE(sf ${SOURCES_SF})
TARGET_LINK_LIBRARIES(sf ${CONTROL} ${COMMON} ${NDDSCPP} ${NDDSC} ${NDDSCORE} ${PT_LIB} -ldl -lrt)

ADD_EXECUTABLE(sl ${SOURCES_SL})
TARGET_LINK_LIBRARIES(sl ${CONTROL} ${COMMON} ${NDDSCPP} ${NDDSC} ${NDDSCORE} ${PT_LIB} -ldl -lrt)

###################

                                                                                                                                                                                                                                                                                                                                ep.cpp                                                                                              0000644 0001750 0001750 00000000733 11467354351 012036  0                                                                                                    ustar   manjesh                         manjesh                                                                                                                                                                                                                #include <stdio.h>
#include <stdlib.h>
#include "ndds_cpp.h"
#include "controlplane.h"

class MANE {
    
public:
    MANE(int appId,int domainId,const char *kind)
    {
        m_appId = appId;
        m_domainId = domainId;
        kind = strdup(kind);
    }
    
    ~MANE()
    {
    }
    
    void PrintInfo()
    {
        printf("MANE[%04d]: domaindId = %d, %s\n",m_appId,m_domainId,kind);
    }
    
private:
    int m_appId;
    int m_domainId;
    char *kind;
};

                                     sf.cpp                                                                                              0000644 0001750 0001750 00000043672 11471271432 012044  0                                                                                                    ustar   manjesh                         manjesh                                                                                                                                                                                                                //!
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
#include <limits.h>
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

                                                                      sl.cpp                                                                                              0000644 0001750 0001750 00000035560 11471271441 012047  0                                                                                                    ustar   manjesh                         manjesh                                                                                                                                                                                                                //!
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
                                                                                                                                                USER_QOS_PROFILES.xml                                                                               0000644 0001750 0001750 00000014440 11475314717 014254  0                                                                                                    ustar   manjesh                         manjesh                                                                                                                                                                                                                <?xml version="1.0"?>
<dds xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
     xsi:noNamespaceSchemaLocation="/Users/tron/RTI/Projects/DDS/BRANCH_NDDS46A/modules/ndds.4.1/scripts/../resource/rtiddsgen/../qos_profiles_4.6a/schema/rti_dds_qos_profiles.xsd"
     version="4.6a">
     
    <qos_library name="NEURON">
    
        <qos_profile name="CP">
                    <!-- Event profile -->
            <datawriter_qos topic_filter="*Event*">
                <reliability>
                    <kind>RELIABLE_RELIABILITY_QOS</kind>
                    <max_blocking_time>
                        <sec>60</sec>
                    </max_blocking_time>
                </reliability>                

                <!-- Make sure all events gets written -->
                <history>
                    <kind>KEEP_ALL_HISTORY_QOS</kind>
                </history>

            </datawriter_qos>

            <datareader_qos topic_filter="*Event*">
                <reliability>
                    <kind>RELIABLE_RELIABILITY_QOS</kind>
                </reliability>                

                <!-- Only store the last 10 events -->
                <history>
                    <kind>KEEP_LAST_HISTORY_QOS</kind>
                    <depth> 10 </depth>
                </history>                
            </datareader_qos>

            <!-- Control profile -->
            <datawriter_qos topic_filter="*Control*">
                <reliability>
                    <kind>RELIABLE_RELIABILITY_QOS</kind>
                    <max_blocking_time>
                        <sec>60</sec>
                    </max_blocking_time>
                </reliability>                

                <history>
                    <kind>KEEP_ALL_HISTORY_QOS</kind>
                </history>

                <resource_limits>
                    <max_samples>50</max_samples>
                </resource_limits>
            </datawriter_qos>
            
            <datareader_qos topic_filter="*Control*">
                <reliability>
                    <kind>RELIABLE_RELIABILITY_QOS</kind>
                </reliability>                

                <!-- Must receive all commands -->
                <history>
                    <kind>KEEP_ALL_HISTORY_QOS</kind>
                </history>
            </datareader_qos>

            <!-- State profile -->
            <datawriter_qos topic_filter="*State*">
                <reliability>
                    <kind>RELIABLE_RELIABILITY_QOS</kind>
                    <max_blocking_time>
                        <sec>60</sec>
                    </max_blocking_time>
                </reliability>                

                <durability>
                    <kind> TRANSIENT_LOCAL_DURABILITY_QOS  </kind>
                </durability>

                <!-- Only keep the most recent state -->
                <history>
                    <kind>KEEP_LAST_HISTORY_QOS</kind>
                    <depth> 1 </depth>
                </history>

                <resource_limits>
                    <max_samples>50</max_samples>
                </resource_limits>
            </datawriter_qos>

            <datareader_qos topic_filter="*State*">
                <reliability>
                    <kind>RELIABLE_RELIABILITY_QOS</kind>
                </reliability>                

                <!-- Only keep the most recent state -->
                <history>
                    <kind>KEEP_LAST_HISTORY_QOS</kind>
                    <depth> 1 </depth>
                </history>

                <durability>
                        <kind> TRANSIENT_LOCAL_DURABILITY_QOS  </kind>
                </durability>
            </datareader_qos>

            <!-- Metrics Profile -->
            <datawriter_qos topic_filter="*Metrics*">
                <reliability>
                    <kind>RELIABLE_RELIABILITY_QOS</kind>
                    <max_blocking_time>
                        <sec>60</sec>
                    </max_blocking_time>
                </reliability>                

                <!-- Make sure all metrics are written -->
                <history>
                    <kind>KEEP_ALL_HISTORY_QOS</kind>
                </history>

            </datawriter_qos>

            <datareader_qos topic_filter="*Metrics*">
                <reliability>
                    <kind>RELIABLE_RELIABILITY_QOS</kind>
                </reliability>                

                <!-- Keep the last 10 metrics -->
                <history>
                    <kind>KEEP_LAST_HISTORY_QOS</kind>
                    <depth> 10 </depth>
                </history>
            </datareader_qos>
            
        </qos_profile>
    
        <!-- No specialization required for the SCP (yet) -->
        <qos_profile name="SCP" base_name="CP">
        
        </qos_profile>

        <!-- No specialization required for the ACP (yet) -->
        <qos_profile name="ACP" base_name="CP">
        
        </qos_profile>
    
        <!-- ECP is only for local communication between the SF and SL
             Thus, only enable shared memory -->
        <qos_profile name="ECP" base_name="CP">
        
            <participant_qos>
                <transport_builtin>
                    <mask> DDS_TRANSPORTBUILTIN_SHMEM </mask>
                </transport_builtin>
                <discovery>
                    <initial_peers> 
                        <element> builtin.shmem:// </element>
                    </initial_peers>
                    <multicast_receive_addresses> 
                    </multicast_receive_addresses>
                </discovery>
            </participant_qos>        
        
        </qos_profile>

        <!-- LSCP is only for local communication between the SF and SL
             Thus, only enable shared memory -->
        <qos_profile name="LSCP" base_name="CP">
            <participant_qos>
                <transport_builtin>
                    <mask> DDS_TRANSPORTBUILTIN_SHMEM </mask>
                </transport_builtin>
                <discovery>
                    <initial_peers> 
                        <element> builtin.shmem://  </element>
                    </initial_peers>
                    <multicast_receive_addresses> 
                    </multicast_receive_addresses>
                </discovery>
            </participant_qos> 
        </qos_profile>

    </qos_library>
</dds>
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                