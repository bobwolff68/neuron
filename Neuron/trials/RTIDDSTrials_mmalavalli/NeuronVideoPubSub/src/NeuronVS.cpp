/*
 * NeuronVS.cpp
 *
 *  Created on: Aug 2, 2010
 *      Author: manjesh
 */

#include <stdio.h>
#include <stdlib.h>
#include "NeuronDDS.h"
#include "NeuronDDSSupport.h"
#include "NeuronVS.h"

//------------------------------------------ NeuronVS member functions ------------------------------------------//

void NeuronVS::setVDSPartition(const char *partitionName)
{
	DDS_SubscriberQos	videoSubQos;
	DDS_ReturnCode_t	retCode;

	retCode = pVideoSub->get_qos(videoSubQos);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to get Frame subscriber QOS\n");
	videoSubQos.partition.name.ensure_length(1,1);
	videoSubQos.partition.name[0] = DDS_String_dup(partitionName);
	retCode = pVideoSub->set_qos(videoSubQos);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to set Frame subscriber QOS\n");
	printf("Frame Subscriber Partition: %s\n",videoSubQos.partition.name[0]);
}

void NeuronVS::setupVDSMulticast(void)
{
	DDS_DataReaderQos	readerQos;
	DDS_ReturnCode_t	retCode;

	retCode = pVideoSub->get_default_datareader_qos(readerQos);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to get video data reader QOS\n");
	readerQos.multicast.value.ensure_length(1,1);
	readerQos.multicast.value[0].receive_address = DDS_String_dup("192.168.46.255");
	retCode = pVideoSub->set_default_datareader_qos(readerQos);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to set video data reader QOS\n");
}

void NeuronVS::startupVideoDataSubscriber(void)
{
	//Create subscriber
    pVideoSub = pDomainParticipant->create_subscriber(DDS_SUBSCRIBER_QOS_DEFAULT,
    												 NULL,DDS_STATUS_MASK_NONE);
    CHECK_HANDLE(pVideoSub,"Unable to create Frame subscriber\n");
    setupVDSMulticast();

    //Create data reader for Frame topic
    gpFrameReader = pVideoSub->create_datareader(pTopic[TOPIC_FRAME],DDS_DATAREADER_QOS_USE_TOPIC_QOS,
            									 NULL,DDS_STATUS_MASK_NONE);
    CHECK_HANDLE(gpFrameReader,"Unable to create data reader for Frame topic\n");

    //Create waitset for Frame topic
    pWaitSet = new DDSWaitSet();

    printf("Created Frame subscriber\n");
}

void NeuronVS::getFrame(unsigned char **ppFrameBuf,int *pBufLen,char layerChoice)
{
	FrameDataReader	   	   *pFrameReader = NULL;
	DDSQueryCondition	   *pWaitSetQueryCond = NULL;
	DDS_Boolean				gotValidData = DDS_BOOLEAN_FALSE;
	const DDS_Duration_t	waitSetTimeout = DDS_DURATION_INFINITE;
	DDSConditionSeq			trigCondList;
	FrameSeq				seqFrames;
	DDS_StringSeq			seqQueryParam(1);
	DDS_SampleInfoSeq		seqSampleInfos;
	DDS_ReturnCode_t		retCode;
	char					paramStr[2] = {layerChoice,'\0'};

	//Create query condition for new samples
	seqQueryParam.ensure_length(1,1);
	seqQueryParam[0] = DDS_String_dup(paramStr);
	pWaitSetQueryCond = gpFrameReader->create_querycondition(DDS_NOT_READ_SAMPLE_STATE,DDS_ANY_VIEW_STATE,
															 DDS_ALIVE_INSTANCE_STATE,"layerType <= %0",
															 seqQueryParam);
	CHECK_HANDLE(pWaitSetQueryCond,"Unable to create query condition for waitset (Frame topic)\n");

	//Attach read condition to waitset
	retCode = pWaitSet->attach_condition(pWaitSetQueryCond);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to attach query condition to waitset (Frame topic)\n");

	//Get pointer to Frame data reader from generic pointer
	pFrameReader = FrameDataReader::narrow(gpFrameReader);
	CHECK_HANDLE(pFrameReader,"FrameDataReader::narrow() error during subscribing\n");

	do
	{
		//Wait until new samples are detected
		retCode = pWaitSet->wait(trigCondList,waitSetTimeout);
		CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Waitset wait error (Frame topic)\n");

		//Take the next new sample
		retCode = pFrameReader->take_w_condition(seqFrames,seqSampleInfos,(DDS_Long)1,pWaitSetQueryCond);

		if(retCode!=DDS_RETCODE_NO_DATA)
		{
			CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Frame data reader take() error\n");
			if(seqSampleInfos[0].valid_data)
			{
				if(*pBufLen<((int) seqFrames[0].payload.length()))
				{
					*ppFrameBuf = (unsigned char *) realloc(*ppFrameBuf,(int) seqFrames[0].payload.length());
					if(*ppFrameBuf==NULL)	printf("Realloc Error\n");
				}
				*pBufLen = (int) seqFrames[0].payload.length();
				seqFrames[0].payload.to_array(reinterpret_cast<DDS_Octet *>(*ppFrameBuf),*pBufLen);
//				printf("Index: %d, LayerType: %d, Size: %d\n",(int)(seqFrames[0].index)
//																 ,(int)(seqFrames[0].layerType)
//																 ,(int)(seqFrames[0].payload.length()));
				gotValidData = DDS_BOOLEAN_TRUE;
			}
		}

	} while(gotValidData==DDS_BOOLEAN_FALSE);

	pFrameReader->return_loan(seqFrames,seqSampleInfos);
	retCode = pWaitSet->detach_condition(pWaitSetQueryCond);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to detach read condition from waitset (Frame topic)\n");
	retCode = gpFrameReader->delete_readcondition(pWaitSetQueryCond);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to delete read condition (Frame topic)\n");
}

NeuronVS::NeuronVS(const char *nameParam) : NeuronDP(nameParam,0,"")
{
	startupVideoDataSubscriber();
}

NeuronVS::~NeuronVS(void)
{
	delete pWaitSet;

	//Destructor of base class NeuronDP is called here
}

//------------------------------------------ NeuronVS APIs ------------------------------------------//

#ifdef __cplusplus
extern "C"
{
#endif

void NVSStartup(const char *name)
{
	pTheVideoSub = new NeuronVS(name);
}

void NVSSetVDSPartition(const char *partitionName)
{
	pTheVideoSub->setVDSPartition(partitionName);
}

void NVSGetFrame(unsigned char **ppFrameBuf,int *pBufLen,char fpsChoice)
{
	char	layerChoice = '2';

	switch(fpsChoice)
	{
		case 'f':	layerChoice = '2';
					break;
		case 'h':	layerChoice = '1';
					break;
		case 'q':	layerChoice = '0';
					break;
	}

	pTheVideoSub->getFrame(ppFrameBuf,pBufLen,layerChoice);
}

void NVSDestroy(void)
{
	delete pTheVideoSub;
}

#ifdef __cplusplus
}
#endif
