/*
 * NeuronDP.cpp
 *
 *  Created on: Jul 30, 2010
 *      Author: manjesh
 */
#include <stdio.h>
#include "NeuronDDS.h"
#include "NeuronDDSSupport.h"
#include "NeuronDP.h"

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

//-------------------- DDSDPBuiltinListener member functions --------------------//

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
        CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Built-in reader take() error\n");
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

//----------------------- NueronDP member functions -----------------------//

void NeuronDP::startupDomainParticipant(void)
{
    DDS_ReturnCode_t                            retCode;
    DDS_DomainParticipantFactoryQos             dpFactoryQos;

    // By default, the domain participant is enabled upon creation. So
    // the automatic enabling has to be turned off so that the listener
    // for built-in topic is attached.
    retCode = DDSTheParticipantFactory->get_qos(dpFactoryQos);
    CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Cannot obtain QOS for domain participant factory\n");
    dpFactoryQos.entity_factory.autoenable_created_entities = DDS_BOOLEAN_FALSE;
    retCode = DDSTheParticipantFactory->set_qos(dpFactoryQos);
    CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Cannot set QOS for domain participant factory\n");

    // Obtain reference to a domain participant from the participant factory
    pDomainParticipant = DDSTheParticipantFactory->create_participant(
                            DEFAULT_DOMAIN_ID,DDS_PARTICIPANT_QOS_DEFAULT,
                            NULL,DDS_STATUS_MASK_NONE);
    CHECK_HANDLE(pDomainParticipant,"Cannot create domain participant\n");
}

void NeuronDP::configParticipantDiscovery(void)
{
    DDS_ReturnCode_t                            retCode;
    DDS_DomainParticipantQos                    dpQos;
    DDSDataReader							   *pGenericReader;
    DDSParticipantBuiltinTopicDataDataReader   *pDPBuiltinReader = NULL;
    DDSDPBuiltinListener                       *pDPBuiltinListener = NULL;

    // Use user_data QOS to advertise identity
    retCode = pDomainParticipant->get_qos(dpQos);
    CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Cannot obtain QOS for domain participant\n");
    dpQos.user_data.value.from_array(reinterpret_cast<const DDS_Octet*>(name),strlen(name)+1);
    retCode = pDomainParticipant->set_qos(dpQos);
    CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Cannot set QOS for domain participant\n");

    // Get participant's built-in subscriber and data reader
    pBuiltinSub = pDomainParticipant->get_builtin_subscriber();
    CHECK_HANDLE(pBuiltinSub,"Cannot obtain domain participant's built-in subscriber\n");
    pGenericReader = pBuiltinSub->lookup_datareader(DDS_PARTICIPANT_TOPIC_NAME);
    CHECK_HANDLE(pGenericReader,"Cannot obtain built-in subscriber's data reader\n");
    pDPBuiltinReader = (DDSParticipantBuiltinTopicDataDataReader *) pGenericReader;

    // Attach listener to built-in data reader and enable domain participant
    pDPBuiltinListener = new DDSDPBuiltinListener();
    pDPBuiltinReader->set_listener(pDPBuiltinListener,DDS_DATA_AVAILABLE_STATUS);
    retCode = pDomainParticipant->enable();
    CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Cannot enable domain participant\n");
    printf("Created domain participant\n" );
}

void NeuronDP::registerAndCreateTopics(void)
{
	const char		   *typeName;
	DDS_ReturnCode_t	retCode;

	for(int iTopic=0; iTopic<N_TOPICS; iTopic++)
	{
		switch(iTopic)
		{
			case TOPIC_FRAME:
					typeName = FrameTypeSupport::get_type_name();
					retCode = FrameTypeSupport::register_type(pDomainParticipant,typeName);
					break;
			case TOPIC_THROTMSG:
					typeName = ThrotMsgTypeSupport::get_type_name();
					retCode = ThrotMsgTypeSupport::register_type(pDomainParticipant,typeName);
					break;
		}

		CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Cannot register type for topic\n");
        printf("Registered type %s\n", typeName);
        pTopic[iTopic] = pDomainParticipant->create_topic(typeName,typeName,
                            DDS_TOPIC_QOS_DEFAULT,NULL,DDS_STATUS_MASK_NONE);
        CHECK_HANDLE(pTopic[iTopic],"Cannot create topic\n");
	}
}

NeuronDP::NeuronDP(const char *nameParam)
{
	name = nameParam;
	startupDomainParticipant();
	configParticipantDiscovery();
	registerAndCreateTopics();
}

NeuronDP::~NeuronDP(void)
{
    DDS_ReturnCode_t    	retCode;

    printf("Shutting down Neuron module...\n");
    retCode = pDomainParticipant->delete_contained_entities();
    CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Can't delete domain participant's entities\n");
    retCode = DDSTheParticipantFactory->delete_participant(pDomainParticipant);
    CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Can't delete domain participant\n");
}
