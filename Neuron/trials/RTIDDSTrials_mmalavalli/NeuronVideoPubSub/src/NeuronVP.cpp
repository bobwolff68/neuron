/*
 * NeuronVP.c
 *
 *  Created on: Jul 30, 2010
 *      Author: manjesh
 */

#include <stdio.h>
#include "NeuronDDS.h"
#include "NeuronDDSSupport.h"
#include "NeuronVP.h"

//------------------------------------ DDSThrotMsgListener member functions ------------------------------------//

void DDSThrotMsgListener::on_data_available(DDSDataReader *pGenericReader)
{
	ThrotMsgDataReader *pThrotMsgReader = NULL;
	ThrotMsgSeq			seqSamples;
	DDS_SampleInfoSeq	seqSampleInfos;
	DDS_ReturnCode_t	retCode;

	pThrotMsgReader = (ThrotMsgDataReader *) pGenericReader;
	retCode = pThrotMsgReader->take(seqSamples,seqSampleInfos,DDS_LENGTH_UNLIMITED,DDS_ANY_SAMPLE_STATE,
									DDS_ANY_VIEW_STATE,DDS_ANY_INSTANCE_STATE);

	if(retCode!=DDS_RETCODE_NO_DATA)
	{
		CHECK_RETCODE(retCode,DDS_RETCODE_OK,"ThrotMsg reader take() error\n");
		if(seqSampleInfos[seqSampleInfos.length()-1].valid_data)
		{
			if(vidSinkName[0]=='\0')
				strcpy(vidSinkName,seqSamples[seqSamples.length()-1].srcName);
			*pTheThrotMode = (int) seqSamples[seqSamples.length()-1].mode;
			//printf("%s: %d\n",vidSinkName,*pTheThrotMode);
		}
	}

	pThrotMsgReader->return_loan(seqSamples,seqSampleInfos);
}

//------------------------------------------ NeuronVP member functions ------------------------------------------//

void NeuronVP::setVDPPartition(const char *partitionName)
{
	DDS_PublisherQos	videoPubQos;
	DDS_ReturnCode_t	retCode;

	retCode = pVideoPub->get_qos(videoPubQos);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to get Frame publisher QOS\n");
	videoPubQos.partition.name.ensure_length(1,1);
	videoPubQos.partition.name[0] = DDS_String_dup(partitionName);
	retCode = pVideoPub->set_qos(videoPubQos);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to set Frame publisher QOS\n");
	printf("Setting VDPPPP Partition: %s\n",videoPubQos.partition.name[0]);
}

void NeuronVP::setTMSPartition(const char *partitionName)
{
	DDS_SubscriberQos	throtSubQos;
	DDS_ReturnCode_t	retCode;

	retCode = pThrotSub->get_qos(throtSubQos);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to get ThrotMsg subscriber QOS\n");
	throtSubQos.partition.name.ensure_length(1,1);
	throtSubQos.partition.name[0] = DDS_String_dup(partitionName);
	retCode = pThrotSub->set_qos(throtSubQos);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to set ThrotMsg subscriber QOS\n");
}

void NeuronVP::startupVideoDataPublisher(void)
{
	FrameDataWriter	   *pFrameWriter = NULL;

	//Create publisher
    pVideoPub = pDomainParticipant->create_publisher(DDS_PUBLISHER_QOS_DEFAULT,
    												 NULL,DDS_STATUS_MASK_NONE);
    CHECK_HANDLE(pVideoPub,"Unable to create Frame publisher\n");

    //Create data writer for Frame topic
    gpFrameWriter = pVideoPub->create_datawriter(pTopic[TOPIC_FRAME],DDS_DATAWRITER_QOS_USE_TOPIC_QOS,
            									 NULL,DDS_STATUS_MASK_NONE);
    CHECK_HANDLE(gpFrameWriter,"Unable to create data writer for Frame topic\n");

    //Register an instance of Frame topic with key as the name of the video source
    pFrameWriter = FrameDataWriter::narrow(gpFrameWriter);
    CHECK_HANDLE(pFrameWriter,"FrameDataWriter::narrow() error during startup\n");
    pFrameInstance = FrameTypeSupport::create_data();
    CHECK_HANDLE(pFrameInstance,"Unable to instantiate new Frame topic instance\n");
    strcpy(pFrameInstance->srcName,name);
    pFrameInstance->index = (DDS_Long) 0;
    hdlFrameInstance = pFrameWriter->register_instance(*pFrameInstance);
    printf("Created Frame publisher\n");
}

void NeuronVP::startupThrotMsgSubscriber(void)
{
	DDSThrotMsgListener	*pThrotMsgListener = new DDSThrotMsgListener();

	//Create ThrotMsg subscriber
	pThrotSub = pDomainParticipant->create_subscriber(DDS_SUBSCRIBER_QOS_DEFAULT,
													  NULL,DDS_STATUS_MASK_NONE);
	CHECK_HANDLE(pThrotSub,"Unable to create ThrotMsg subscriber");

	//Set subscriber partition to video source name
	setTMSPartition(name);

	//Create data reader for ThrotMsg topic and add listener
	gpThrotReader = pThrotSub->create_datareader(pTopic[TOPIC_THROTMSG],DDS_DATAREADER_QOS_DEFAULT,
												 NULL,DDS_STATUS_MASK_NONE);
	CHECK_HANDLE(gpThrotReader,"Unable to create data reader for ThrotMsg topic\n");
	gpThrotReader->set_listener(pThrotMsgListener,DDS_DATA_AVAILABLE_STATUS);
	printf("Created ThrotMsg subscriber\n");
}

void NeuronVP::publishFrame(unsigned char *pFrameBuf,int bufLen,int lType)
{
	DDS_ReturnCode_t	retCode;
	FrameDataWriter	   *pFrameWriter = NULL;

	pFrameInstance->layerType = (DDS_Long) lType;
	pFrameInstance->payload.from_array(reinterpret_cast<const DDS_Octet *>(pFrameBuf),bufLen);
	pFrameWriter = FrameDataWriter::narrow(gpFrameWriter);
	CHECK_HANDLE(pFrameWriter,"FrameDataWriter::narrow() error during Frame publishing\n");
	retCode = pFrameWriter->write(*pFrameInstance,hdlFrameInstance);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to write Frame topic sample\n");
	printf("Index: %d, LayerType: %d, Size: %d\n",(int)(pFrameInstance->index)
												 ,(int)(pFrameInstance->layerType)
												 ,(int)(pFrameInstance->payload.length()));
	pFrameInstance->index++;
}

NeuronVP::NeuronVP(const char *nameParam) : NeuronDP(nameParam,1)
{
	startupVideoDataPublisher();
	startupThrotMsgSubscriber();
}

NeuronVP::~NeuronVP(void)
{
	DDS_ReturnCode_t	retCode;
	FrameDataWriter	   *pFrameWriter = NULL;

	//Unregister instance of Frame topic
	pFrameWriter = FrameDataWriter::narrow(gpFrameWriter);
    CHECK_HANDLE(pFrameWriter,"FrameDataWriter::narrow() error during shutdown\n");
	retCode = pFrameWriter->unregister_instance(*pFrameInstance,hdlFrameInstance);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to unregister Frame instance\n");

	//Delete instance of Frame topic
	retCode = FrameTypeSupport::delete_data(pFrameInstance);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to delete Frame topic instance\n");

	//Destructor of base class NeuronDP is called here
}

//------------------------------------------ NeuronVP APIs ------------------------------------------//

#ifdef __cplusplus
extern "C"
{
#endif

void NVPStartup(const char *name)
{
	pTheVideoPub = new NeuronVP(name);
	pTheThrotMode = NULL;
	vidSinkName[0] = '\0';
}

void NVPSetThrotModePtr(int *pThrotMode)
{
	pTheThrotMode = pThrotMode;
}

void NVPSetVDPPartition(void)
{
	pTheVideoPub->setVDPPartition(vidSinkName);
}

void NVPPublishFrame(unsigned char *pFrameBuf,int bufLen,int lType)
{
	pTheVideoPub->publishFrame(pFrameBuf,bufLen,lType);
}

void NVPDestroy(void)
{
	delete pTheVideoPub;
}

#ifdef __cplusplus
}
#endif
