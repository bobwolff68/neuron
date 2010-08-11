/*
 * NeuronDP.cpp
 *
 *  Created on: Jul 30, 2010
 *      Author: manjesh
 */
#include <stdio.h>
#include <string.h>
#include "NeuronDDS.h"
#include "NeuronDDSSupport.h"
#include "NeuronDP.h"

//-------------------- DDSDPBuiltinListener member functions --------------------//

void DDSDPBuiltinListener::on_data_available(DDSDataReader *pGenericReader)
{
    DDSParticipantBuiltinTopicDataDataReader   *dpBuiltinReader = NULL;
    DDS_ParticipantBuiltinTopicDataSeq          seqDiscovery;
    DDS_SampleInfoSeq                           seqInfo;
    DDS_ReturnCode_t                            retCode;
    char                                       *newUserInfo = NULL;
    int											fVidSrc;
    int											vidWidth;
    int											vidHeight;
    double										vidFps;

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
                    newUserInfo = (char *) &(seqDiscovery[iSeq].user_data.value[0]);
                    fVidSrc = (int) (newUserInfo[0]-'0');

                    if(fVidSrc)
                    {
                    	sscanf(&newUserInfo[2],"%d,%d,%lf,%s",&vidWidth,&vidHeight,&vidFps,srcNameList[srcNameListLen]);
                    	sprintf(srcVidStats[srcNameListLen++],"(%dX%d)@(%.2lf)",vidWidth,vidHeight,vidFps);
                    }
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

void NeuronDP::configParticipantDiscovery(int fVidSrc,const char *vidStats)
{
	char									   *discInfo;
    char										fVidSrcInfo[3];
	DDS_ReturnCode_t                            retCode;
    DDS_DomainParticipantQos                    dpQos;
    DDSDataReader							   *pGenericReader;
    DDSParticipantBuiltinTopicDataDataReader   *pDPBuiltinReader = NULL;
    DDSDPBuiltinListener                       *pDPBuiltinListener = NULL;

    discInfo = new char[MAX_NAME_LEN+50];
    //strcpy(discInfo,name);
    sprintf(fVidSrcInfo,"%d,",fVidSrc);
    strcpy(discInfo,fVidSrcInfo);
    strcat(discInfo,vidStats);
    strcat(discInfo,",");
    strcat(discInfo,name);

    // Use user_data QOS to advertise identity
    retCode = pDomainParticipant->get_qos(dpQos);
    CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Cannot obtain QOS for domain participant\n");
    dpQos.user_data.value.from_array(reinterpret_cast<const DDS_Octet*>(discInfo),strlen(discInfo)+1);
    retCode = pDomainParticipant->set_qos(dpQos);
    CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Cannot set QOS for domain participant\n");
    delete discInfo;

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
	DDS_TopicQos		frameTopicQos;

	for(int iTopic=0; iTopic<N_TOPICS; iTopic++)
	{
		switch(iTopic)
		{
			case TOPIC_FRAME:
					typeName = FrameTypeSupport::get_type_name();
					retCode = FrameTypeSupport::register_type(pDomainParticipant,typeName);
					CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Cannot register type for Frame topic\n");
					// Make frame data reliable and history depth 2
					retCode = pDomainParticipant->get_default_topic_qos(frameTopicQos);
					CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Cannot get QOS for Frame topic\n");
					frameTopicQos.history.depth = 2;
					retCode = pDomainParticipant->set_default_topic_qos(frameTopicQos);
					CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Cannot set QOS for Frame topic\n");
					break;
		}

        printf("Registered type %s\n", typeName);
        pTopic[iTopic] = pDomainParticipant->create_topic(typeName,typeName,
                            DDS_TOPIC_QOS_DEFAULT,NULL,DDS_STATUS_MASK_NONE);
        CHECK_HANDLE(pTopic[iTopic],"Cannot create topic\n");
	}
}

NeuronDP::NeuronDP(const char *nameParam,int fVidSrc,const char *vidStats)
{
	name = nameParam;
	srcNameListLen = 0;
	startupDomainParticipant();
	configParticipantDiscovery(fVidSrc,vidStats);
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
