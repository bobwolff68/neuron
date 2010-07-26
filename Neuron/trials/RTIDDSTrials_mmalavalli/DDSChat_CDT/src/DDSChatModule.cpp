#include <stdio.h>
#include "DDSChat.h"
#include "DDSChatSupport.h"
#include "DDSChatModule.h"

#define CHECK_HANDLE(pHandle,errMsg) if(pHandle==NULL)\
                                       {\
                                           printf(errMsg);\
                                           exit(0);\
                                       }
#define CHECK_RETCODE(retCode,value,errMsg) if(retCode!=value)\
                                            {\
                                                printf(errMsg);\
                                                exit(0);\
                                            }

void DDSDPBuiltinListener::on_data_available(DDSDataReader *pGenericReader)
{
    DDSParticipantBuiltinTopicDataDataReader   *dpBuiltinReader = NULL;
    DDS_ParticipantBuiltinTopicDataSeq          seqDiscovery;
    DDS_SampleInfoSeq                           seqInfo;
    DDS_ReturnCode_t                            retCode;
    char                                       *userData = NULL;
    char                                       *newUserName = new char [100];
    long                                        newUserId;

    dpBuiltinReader = (DDSParticipantBuiltinTopicDataDataReader *) pGenericReader;
    retCode = dpBuiltinReader->take(seqDiscovery,seqInfo,DDS_LENGTH_UNLIMITED,
                DDS_ANY_SAMPLE_STATE,DDS_NEW_VIEW_STATE,DDS_ANY_INSTANCE_STATE);

    if(retCode!=DDS_RETCODE_NO_DATA)
    {
        CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Builtin reader take() error\n");
        for(int iSeq=0; iSeq<seqDiscovery.length(); iSeq++)
        {
            if(seqInfo[iSeq].valid_data)
            {
                if(seqDiscovery[iSeq].user_data.value.length()>0)
                {
                    userData = (char *) &(seqDiscovery[iSeq].user_data.value[0]);
                    sscanf(userData,"%ld,%s",&newUserId,newUserName);
                    printf("%s (User ID: %ld) has come online...\n",newUserName,newUserId);
                }
            }
        }
    }

    dpBuiltinReader->return_loan(seqDiscovery,seqInfo);
    delete [] newUserName;
}

void DDSChatModule::startupDomainParticipant(int domainId)
{
    char                                       *clientIdentity = NULL;
    DDS_ReturnCode_t                            retCode;
    DDS_DomainParticipantFactoryQos             dpFactoryQos;
    DDS_DomainParticipantQos                    dpQos;
    DDSParticipantBuiltinTopicDataDataReader   *dpBuiltinReader = NULL;
    DDSDPBuiltinListener                       *dpBuiltinListener = NULL;

    // By default, the domain participant is enabled upon creation. So
    // the automatic enabling has to be turned off so that the listener
    // for builtin topic is attached.
    retCode = DDSTheParticipantFactory->get_qos(dpFactoryQos);
    CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Cannot obtain QOS for domain participant factory\n");
    dpFactoryQos.entity_factory.autoenable_created_entities = DDS_BOOLEAN_FALSE;
    retCode = DDSTheParticipantFactory->set_qos(dpFactoryQos);
    CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Cannot set QOS for domain participant factory\n");

    // Obtain refrence to a domain participant from the participant factory
    pDomainParticipant = DDSTheParticipantFactory->create_participant(
                            domainId,DDS_PARTICIPANT_QOS_DEFAULT,
                            NULL, DDS_STATUS_MASK_NONE);
    CHECK_HANDLE(pDomainParticipant,"Cannot create domain participant\n");

    // Use user_data QOS to advertise identity
    retCode = pDomainParticipant->get_qos(dpQos);
    CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Cannot obtain QOS for domain participant\n");
    clientIdentity = new char [100];
    sprintf(clientIdentity,"%ld,%s",id,name);
    dpQos.user_data.value.from_array(reinterpret_cast<const DDS_Octet*>(clientIdentity),
                                     strlen(clientIdentity)+1); //Room for '\0'
    retCode = pDomainParticipant->set_qos(dpQos);
    CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Cannot set QOS for domain participant\n");
    delete [] clientIdentity;

    // Get participant's builtin subscriber and datareader
    pBuiltinSub = pDomainParticipant->get_builtin_subscriber();
    CHECK_HANDLE(pBuiltinSub,"Cannot obtain domain participant's builtin subscriber\n");
    pReader[TOPIC_DISCOVERY] = pBuiltinSub->lookup_datareader(DDS_PARTICIPANT_TOPIC_NAME);
    CHECK_HANDLE(pReader[TOPIC_DISCOVERY],"Cannot obtain builtin subscriber's datareader\n");
    dpBuiltinReader = (DDSParticipantBuiltinTopicDataDataReader *) pReader[TOPIC_DISCOVERY];

    // Attach listener to builtin datareader and enable domain participant
    dpBuiltinListener = new DDSDPBuiltinListener();
    dpBuiltinReader->set_listener(dpBuiltinListener,DDS_DATA_AVAILABLE_STATUS);
    retCode = pDomainParticipant->enable();
    CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Cannot attach listener to builtin reader\n");
    printf("Created domain participant\n" );

    // Register types for respective topics and then create the topics
    for(int iTopic=0; iTopic<(MAX_TOPICS-1); iTopic++)  // pTopi[MAX_TOPICS-1] is builtin publisher topic
    {
        pCChar typeName = ChatMessageTypeSupport::get_type_name();
        retCode = ChatMessageTypeSupport::register_type(pDomainParticipant,typeName);
        CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Cannot register type for topic\n");
        printf("Registered type %s\n", typeName);

        pTopic[iTopic] = pDomainParticipant->create_topic(typeName,typeName,
                            DDS_TOPIC_QOS_DEFAULT,NULL,DDS_STATUS_MASK_NONE);
        CHECK_HANDLE(pTopic[iTopic],"Cannot create topic\n");
    }
}

void DDSChatModule::startupPublisher(void)
{
    // Create publisher and set user data as "<name>,<id>"
    pPub = pDomainParticipant->create_publisher(DDS_PUBLISHER_QOS_DEFAULT,NULL,
            DDS_STATUS_MASK_NONE);
    CHECK_HANDLE(pPub,"Cannot create publisher\n");
    printf("Created publisher\n");


    // Create data writers for respective topics
    for(int iTopic=0; iTopic<(MAX_TOPICS-1); iTopic++)
    {
        pWriter[iTopic] = pPub->create_datawriter(pTopic[iTopic],DDS_DATAWRITER_QOS_DEFAULT,
                    NULL,DDS_STATUS_MASK_NONE);
        CHECK_HANDLE(pWriter[iTopic],"Cannot create data writer\n");
    }
}

void DDSChatModule::startupSubscriber(void)
{
    // Create subscriber
    pSub = pDomainParticipant->create_subscriber(DDS_SUBSCRIBER_QOS_DEFAULT,NULL,
            DDS_STATUS_MASK_NONE);
    CHECK_HANDLE(pSub,"Cannot create publisher\n");
    printf("Created subscriber\n");

    // Create data readers for respective topics
    for(int iTopic=0; iTopic<(MAX_TOPICS-1); iTopic++)
    {
        pReader[iTopic] = pSub->create_datareader(pTopic[iTopic],DDS_DATAREADER_QOS_DEFAULT,
                    NULL,DDS_STATUS_MASK_NONE);
        CHECK_HANDLE(pReader[iTopic],"Cannot create data reader\n");
    }
}

void DDSChatModule::startup(void)
{
    printf("Starting up chat module...\n");
    startupDomainParticipant(DEFAULT_DOMAIN_ID);
    startupPublisher();
    startupSubscriber();
    printf("Startup complete...\n");
}

DDSChatModule::~DDSChatModule()
{
    DDS_ReturnCode_t    retCode;

    printf("Shutting down chat module...\n");
    retCode = pDomainParticipant->delete_contained_entities();
    CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Can't delete domain participant's entities\n");
    retCode = DDSTheParticipantFactory->delete_participant(pDomainParticipant);
    CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Can't delete domain participant\n");
}
