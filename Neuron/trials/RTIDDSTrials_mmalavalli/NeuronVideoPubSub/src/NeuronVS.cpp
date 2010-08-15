/*
 * NeuronVS.cpp
 *
 *  Created on: Aug 2, 2010
 *      Author: manjesh
 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "NeuronDDS.h"
#include "NeuronDDSSupport.h"
#include "NeuronVS.h"

//-------------------------------------- FrameDataListener member functions -------------------------------------//

void FrameDataListener::on_data_available(DDSDataReader *pGenericReader)
{
	FrameDataReader	   	   *pFrameReader = NULL;
	FrameSeq				seqFrames;
	DDS_SampleInfoSeq		seqSampleInfos;
	DDS_ReturnCode_t		retCode;
	DDS_DataReaderQos		frameReaderQos;
	void				   *pFrameBuf = NULL;
	int						lenFrameBuf;
	int						fid;

	//Obtain pointer to frame data reader from generic reader
	pFrameReader = FrameDataReader::narrow(pGenericReader);
	CHECK_HANDLE(pFrameReader,"FrameDataReader::narrow() error\n");

	//Obtain fid of frameOutFifo from reader's property qos
	retCode = pGenericReader->get_qos(frameReaderQos);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to get frame reader QOS\n");
	sscanf(frameReaderQos.property.value[0].value,"%d",&fid);

	//Collect new samples
	retCode = pFrameReader->take(seqFrames,seqSampleInfos,DDS_LENGTH_UNLIMITED,
								 DDS_NOT_READ_SAMPLE_STATE,DDS_ANY_VIEW_STATE,
								 DDS_ALIVE_INSTANCE_STATE);
	if(retCode!=DDS_RETCODE_NO_DATA)
	{
		CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Frame data reader take() error\n");
		for(int iSeq=0; iSeq<seqFrames.length(); iSeq++)
		{
			if(seqSampleInfos[iSeq].valid_data)
			{
				pFrameBuf = reinterpret_cast<void *>(seqFrames[iSeq].payload.get_contiguous_buffer());
				lenFrameBuf = (int) seqFrames[iSeq].payload.length();
				if(write(fid,(void *)(&lenFrameBuf),sizeof(int))!=sizeof(int))
					printf("Failed to write frame (Src: %s, Index: %lld, LType: %lld, Size: %d",
						   seqFrames[iSeq].srcName, (long long)seqFrames[iSeq].index,
						   (long long)seqFrames[iSeq].layerType,lenFrameBuf);
				if(write(fid,pFrameBuf,lenFrameBuf)!=lenFrameBuf)
					printf("Failed to write frame (Src: %s, Index: %lld, LType: %lld, Size: %d",
						   seqFrames[iSeq].srcName, (long long)seqFrames[iSeq].index,
						   (long long)seqFrames[iSeq].layerType,lenFrameBuf);
			}
		}
	}

	pFrameReader->return_loan(seqFrames,seqSampleInfos);
}

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

void NeuronVS::setupFrameListener(void)
{
	FrameDataListener  *pFrameListener = NULL;
	FrameDataReader	   *pFrameReader = NULL;
	DDS_DataReaderQos	frameReaderQos;
	DDS_ReturnCode_t	retCode;
	char				frameOutFifo[2*MAX_NAME_LEN] = "../../Fifos/x264vfrmin";
	char				fidStr[10];
	int					fid;

	//Attach frame listener to reader
	pFrameReader = FrameDataReader::narrow(gpFrameReader);
	CHECK_HANDLE(pFrameReader,"FrameDataReader::narrow() error in setupFrameListener()\n");
	pFrameListener = new FrameDataListener();
	pFrameReader->set_listener(pFrameListener,DDS_DATA_AVAILABLE_STATUS);

	//Open frame output fifo that is read by the VFRM (Variable Frame Rate Module) thread
	strcat(frameOutFifo,name);
	strcat(frameOutFifo,".264");
	if((fid = open(frameOutFifo,O_WRONLY))<0)
		printf("Error opening fifo: %s\n", frameOutFifo);
	else
	{
		//Add property (frameOutFifo,fid) to reader so fid can be accessed by listener
		sprintf(fidStr,"%d",fid);
		retCode = gpFrameReader->get_qos(frameReaderQos);
		CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to get frame reader QOS\n");
		retCode = DDSPropertyQosPolicyHelper::add_property(frameReaderQos.property,"frameOutFifo",
												 	 	   fidStr,DDS_BOOLEAN_FALSE);
		CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to add property\n");
		retCode = gpFrameReader->set_qos(frameReaderQos);
		CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to set frame reader QOS\n");
	}
}

void NeuronVS::changeVDSPartition(char layerChoice)
{
	DDS_SubscriberQos	videoSubQos;
	DDS_ReturnCode_t	retCode;
	int					lenPartitionName;

	retCode = pVideoSub->get_qos(videoSubQos);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to get Frame subscriber QOS\n");
	lenPartitionName = strlen(videoSubQos.partition.name[0]);
	videoSubQos.partition.name[0][lenPartitionName-1] = (char) ('0' + (2 - (int)(layerChoice-'0')));
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
	readerQos.multicast.value[0].receive_address = DDS_String_dup("192.168.46.255"/*"239.255.1.2"*/);
	retCode = pVideoSub->set_default_datareader_qos(readerQos);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to set video data reader QOS\n");
}

void NeuronVS::startupVideoDataSubscriber(void)
{
	//Create subscriber
    pVideoSub = pDomainParticipant->create_subscriber(DDS_SUBSCRIBER_QOS_DEFAULT,
    												 NULL,DDS_STATUS_MASK_NONE);
    CHECK_HANDLE(pVideoSub,"Unable to create Frame subscriber\n");
    //setupVDSMulticast();

    //Create data reader and listener for Frame topic
    gpFrameReader = pVideoSub->create_datareader(pTopic[TOPIC_FRAME],DDS_DATAREADER_QOS_USE_TOPIC_QOS,
            									 NULL,DDS_STATUS_MASK_NONE);
    CHECK_HANDLE(gpFrameReader,"Unable to create data reader for Frame topic\n");
    printf("Created Frame subscriber\n");
}

NeuronVS::NeuronVS(const char *nameParam) : NeuronDP(nameParam,0,"")
{
	startupVideoDataSubscriber();
}

NeuronVS::~NeuronVS(void)
{
	DDS_DataReaderQos	frameReaderQos;
	DDS_ReturnCode_t	retCode;
	int					fid;

	//Close frameOutFifo
	retCode = gpFrameReader->get_qos(frameReaderQos);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to get frame reader QOS\n");
	sscanf(frameReaderQos.property.value[0].value,"%d",&fid);
	close(fid);

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

void NVSSetupFrameListener(void)
{
	pTheVideoSub->setupFrameListener();
}

void NVSSetVDSPartition(const char *partitionName)
{
	char	partitionStr[2*MAX_NAME_LEN];

	strcpy(partitionStr,partitionName);
	strcat(partitionStr,"/L0");
	pTheVideoSub->setVDSPartition(partitionStr);
}

void NVSChangeVDSPartition(char fpsChoice)
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

	pTheVideoSub->changeVDSPartition(layerChoice);
}

void NVSDestroy(void)
{
	delete pTheVideoSub;
}

#ifdef __cplusplus
}
#endif
