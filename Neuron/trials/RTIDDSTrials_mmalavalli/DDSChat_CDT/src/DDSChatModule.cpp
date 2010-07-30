#include <stdio.h>
#include <unistd.h>
#include <string.h>
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

#define DDSCHAT_PROMPT(cliName)	"%s@DDSChat>",cliName

void DDSDPBuiltinListener::on_data_available(DDSDataReader *pGenericReader)
{
    DDSParticipantBuiltinTopicDataDataReader   *dpBuiltinReader = NULL;
    DDS_ParticipantBuiltinTopicDataSeq          seqDiscovery;
    DDS_SampleInfoSeq                           seqInfo;
    DDS_ReturnCode_t                            retCode;
    char                                       *newUserName = NULL;

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
                    newUserName = (char *) &(seqDiscovery[iSeq].user_data.value[0]);
                    printf("\nOnline: %s\n",newUserName);
                }
            }
        }
    }

    dpBuiltinReader->return_loan(seqDiscovery,seqInfo);
}

void ChatMsgListener::on_data_available(DDSDataReader *pGenericReader)
{
	ChatMessageDataReader  *pChatMsgReader = NULL;
	ChatMessageSeq			seqSamples;
	DDS_SampleInfoSeq		seqInfo;
	DDS_ReturnCode_t		retCode;

	pChatMsgReader = (ChatMessageDataReader *) pGenericReader;
	retCode = pChatMsgReader->take(seqSamples,seqInfo,DDS_LENGTH_UNLIMITED,
				DDS_ANY_SAMPLE_STATE,DDS_ANY_VIEW_STATE,DDS_ANY_INSTANCE_STATE);

    if(retCode!=DDS_RETCODE_NO_DATA)
    {
        CHECK_RETCODE(retCode,DDS_RETCODE_OK,"ChatMessage reader take() error\n");
        for(int iSeq=0; iSeq<seqSamples.length(); iSeq++)
        {
            if(seqInfo[iSeq].valid_data)
            	printf("\n%s: %s\n",seqSamples[iSeq].srcCliName,seqSamples[iSeq].content);
            else if(seqInfo[iSeq].instance_state==DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE ||
            		seqInfo[iSeq].instance_state==DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE)
            {
            	if(DDS_InstanceHandle_is_nil(&(seqInfo[iSeq].instance_handle))==DDS_BOOLEAN_FALSE)
            	{
            		ChatMessage	*pKeyHolderInstance = NULL;
            		pKeyHolderInstance = ChatMessageTypeSupport::create_data();
            		CHECK_HANDLE(pKeyHolderInstance,"Unable to instantiate ChatMessage Topic instance\n");
            		retCode = pChatMsgReader->get_key_value(*pKeyHolderInstance,seqInfo[iSeq].instance_handle);
            		CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to extract key from instance handle\n");
            		printf("\nOffline: %s\n",pKeyHolderInstance->srcCliName);
            		retCode = ChatMessageTypeSupport::delete_data(pKeyHolderInstance);
            		CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to delete ChatMessage Topic instance\n");
            	}
            }
        }
    }

    pChatMsgReader->return_loan(seqSamples,seqInfo);
}

/*void DDSChatModule::regunregChatMsgInstance(DDS_Boolean regFlag)
{
	DDS_ReturnCode_t		retCode;
    ChatMessage		   	   *pChatMsgInstance = NULL;
    ChatMessageDataWriter  *pChatMsgWriter = NULL;

    //Register ChatMessage Topic instance for current source
	pChatMsgInstance = ChatMessageTypeSupport::create_data();
	CHECK_HANDLE(pChatMsgInstance,"Unable to instantiate ChatMessage Topic instance\n");
	strcpy(pChatMsgInstance->srcCliName,name);
	pChatMsgWriter = ChatMessageDataWriter::narrow(pWriter[TOPIC_MSG]);
	CHECK_HANDLE(pChatMsgWriter,"ChatMessageDataWriter::narrow() error\n");
	if(regFlag)
		chatMsgInstHdl = pChatMsgWriter->register_instance(*pChatMsgInstance);
	else
	{
		retCode = pChatMsgWriter->unregister_instance(*pChatMsgInstance,chatMsgInstHdl);
		CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to unregister ChatMessage Topic instance");
	}
	retCode = ChatMessageTypeSupport::delete_data(pChatMsgInstance);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to delete ChatMessage Topic instance\n");
}*/

void DDSChatModule::startupDomainParticipant(int domainId)
{
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
    dpQos.user_data.value.from_array(reinterpret_cast<const DDS_Octet*>(name),strlen(name)+1);
    retCode = pDomainParticipant->set_qos(dpQos);
    CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Cannot set QOS for domain participant\n");

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

    // Register ChatMessage instance for this source
    //regunregChatMsgInstance(DDS_BOOLEAN_TRUE);
}

void DDSChatModule::startupSubscriber(void)
{
	DDS_ReturnCode_t		retCode;
	DDS_SubscriberQos		subQos;
	ChatMsgListener	   	   *pChatListener = NULL;

    // Create subscriber
    pSub = pDomainParticipant->create_subscriber(DDS_SUBSCRIBER_QOS_DEFAULT,NULL,
            DDS_STATUS_MASK_NONE);
    CHECK_HANDLE(pSub,"Cannot create publisher\n");
    printf("Created subscriber\n");

	//Set subscriber partition name to client name
	retCode = pSub->get_qos(subQos);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to get subscriber QOS\n");
	subQos.partition.name.ensure_length(1,1);
	subQos.partition.name[0] = DDS_String_dup(name);
	retCode = pSub->set_qos(subQos);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to set subscriber QOS\n");

    // Create data readers for respective topics
    for(int iTopic=0; iTopic<(MAX_TOPICS-1); iTopic++)
    {
        pReader[iTopic] = pSub->create_datareader(pTopic[iTopic],DDS_DATAREADER_QOS_DEFAULT,
                    NULL,DDS_STATUS_MASK_NONE);
        CHECK_HANDLE(pReader[iTopic],"Cannot create data reader\n");
    }

    // Attach listener to ChatMessage Topic data reader
    pChatListener = new ChatMsgListener();
    pReader[TOPIC_MSG]->set_listener(pChatListener,DDS_DATA_AVAILABLE_STATUS);
}

void DDSChatModule::sendChatMessage(char *cliName,char *msgContent)
{
	DDS_ReturnCode_t		retCode;
	DDS_PublisherQos		pubQos;
	ChatMessage			   *pChatMsgInstance = NULL;
	ChatMessageDataWriter  *pChatMsgWriter = NULL;

	//Set publisher's partition name to match that of destination's subscriber
	retCode = pPub->get_qos(pubQos);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to get publisher QOS\n");
	pubQos.partition.name.ensure_length(1,1);
	pubQos.partition.name[0] = DDS_String_dup(cliName);
	retCode = pPub->set_qos(pubQos);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to set publisher QOS\n");
	usleep(20000); //Time for DDS to propagate partition change info to other clients

	//Obtain data writer specific to ChatMessage Topic
	pChatMsgWriter = ChatMessageDataWriter::narrow(pWriter[TOPIC_MSG]);
	CHECK_HANDLE(pChatMsgWriter,"ChatMessageDataWriter::narrow() error\n");

	//Instantiate an instance of ChatMessage Topic
	pChatMsgInstance = ChatMessageTypeSupport::create_data();
	CHECK_HANDLE(pChatMsgInstance,"Unable to instantiate ChatMessage Topic instance\n");

	//Fill instance sample with provided data
	strcpy(pChatMsgInstance->srcCliName,name);
	strcpy(pChatMsgInstance->content,msgContent);

	//Write sample to destination
	retCode = pChatMsgWriter->write(*pChatMsgInstance,DDS_HANDLE_NIL);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to send chat message\n");

	//Delete instance
	retCode = ChatMessageTypeSupport::delete_data(pChatMsgInstance);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to delete ChatMessage Topic instance\n");
}

void DDSChatModule::performTask(void)
{
	char	cmdLineText[MAX_CMDLINE_LEN] = "";

	do
	{
		printf(DDSCHAT_PROMPT(name));
		fgets(cmdLineText,MAX_CMDLINE_LEN,stdin);
		cmdLineText[strlen(cmdLineText)-1] = '\0'; //Overwrite '\n' with '\0'

		if(!strcmp(cmdLineText,"cmdlist"))
		{
			printf("List of available commands:\n");
			printf("1. cmdlist (list of commands)\n");
			printf("2. @<clientname>\\n<chatmessage> (send <chatmessage> to <clientname>)\n");
			printf("3. logout (log out of chat network)\n");
		}
		else if(cmdLineText[0]=='@')
		{
			char destCliName[MAX_NAME_LEN+1];
			char chatMsgContent[MAX_MSG_LEN+1];

			sscanf(cmdLineText,"@%s",destCliName);
			printf("Message: ");
			fgets(chatMsgContent,MAX_MSG_LEN,stdin);
			chatMsgContent[strlen(chatMsgContent)-1] = '\0'; //Overwrite '\n' with '\0'
			sendChatMessage(destCliName,chatMsgContent);
			printf("Message sent to %s\n",destCliName);
		}
		else if(strcmp(cmdLineText,"")&&strcmp(cmdLineText,"logout"))
			printf("Unknown command\n");
	} while(strcmp(cmdLineText,"logout"));
}

void DDSChatModule::startup(void)
{
    printf("Starting up chat module...\n");
    startupDomainParticipant(DEFAULT_DOMAIN_ID);
    startupPublisher();
    startupSubscriber();
    printf("Startup complete...type 'cmdlist' for available commands\n");
    performTask();
}

DDSChatModule::~DDSChatModule()
{
    DDS_ReturnCode_t    	retCode;

    // Unregister ChatMessage instance for this source
    //regunregChatMsgInstance(DDS_BOOLEAN_FALSE);

    printf("Shutting down chat module...\n");
    retCode = pDomainParticipant->delete_contained_entities();
    CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Can't delete domain participant's entities\n");
    retCode = DDSTheParticipantFactory->delete_participant(pDomainParticipant);
    CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Can't delete domain participant\n");
}
