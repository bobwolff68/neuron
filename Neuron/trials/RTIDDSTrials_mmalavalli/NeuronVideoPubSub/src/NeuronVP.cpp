/*
 * NeuronVP.c
 *
 *  Created on: Jul 30, 2010
 *      Author: manjesh
 */

#include <stdio.h>
#include <string.h>
#include "NeuronDDS.h"
#include "NeuronDDSSupport.h"
#include "NeuronVP.h"

//------------------------------------------ NeuronVP member functions ------------------------------------------//

void NeuronVP::setVDPPartition(int pubIndex,const char *partitionName)
{
	DDS_PublisherQos	videoPubQos;
	DDS_ReturnCode_t	retCode;

	retCode = pVideoPub[pubIndex]->get_qos(videoPubQos);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to get Frame publisher QOS\n");
	videoPubQos.partition.name.ensure_length(1,1);
	videoPubQos.partition.name[0] = DDS_String_dup(partitionName);
	retCode = pVideoPub[pubIndex]->set_qos(videoPubQos);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to set Frame publisher QOS\n");
	printf("Setting Publisher Partition: %s\n",videoPubQos.partition.name[0]);
}

void NeuronVP::startupVideoDataPublisher(void)
{
	char				layerStr[3] = {'L','0','\0'};
	char				partitionName[2*MAX_NAME_LEN];
	FrameDataWriter	   *pFrameWriter = NULL;

	pFrameInstance = FrameTypeSupport::create_data();
	CHECK_HANDLE(pFrameInstance,"Unable to instantiate new Frame topic instance\n");
	strcpy(pFrameInstance->srcName,name);
	pFrameInstance->index = (DDS_Long) 0;

	//Create publishers
	for(int iPub=0; iPub<3; iPub++)
	{
		pVideoPub[iPub] = pDomainParticipant->create_publisher(DDS_PUBLISHER_QOS_DEFAULT,
															   NULL,DDS_STATUS_MASK_NONE);
		CHECK_HANDLE(pVideoPub[iPub],"Unable to create Frame publisher\n");

		//Set publisher's partition name to it's name/layer
		layerStr[1] = '0' + iPub;
		strcpy(partitionName,name);
		strcat(partitionName,"/");
		strcat(partitionName,layerStr);
		setVDPPartition(iPub,partitionName);

		//Create data writer for Frame topic
		gpFrameWriter[iPub] = pVideoPub[iPub]->create_datawriter(pTopic[TOPIC_FRAME],
												    DDS_DATAWRITER_QOS_USE_TOPIC_QOS,
												    NULL,DDS_STATUS_MASK_NONE);
		CHECK_HANDLE(gpFrameWriter[iPub],"Unable to create data writer for Frame topic\n");

		//Register an instance of Frame topic with key as the name of the video source
		pFrameWriter = FrameDataWriter::narrow(gpFrameWriter[iPub]);
		CHECK_HANDLE(pFrameWriter,"FrameDataWriter::narrow() error during startup\n");

		hdlFrameInstance[iPub] = pFrameWriter->register_instance(*pFrameInstance);
		printf("Created Frame publisher\n");
	}
}

void NeuronVP::publishFrame(unsigned char *pFrameBuf,int bufLen,int lType)
{
	DDS_ReturnCode_t	retCode;
	FrameDataWriter	   *pFrameWriter = NULL;

	pFrameInstance->layerType = (DDS_Long) lType;
	pFrameInstance->payload.from_array(reinterpret_cast<const DDS_Octet *>(pFrameBuf),bufLen);
	pFrameWriter = FrameDataWriter::narrow(gpFrameWriter[2-lType]);
	CHECK_HANDLE(pFrameWriter,"FrameDataWriter::narrow() error during Frame publishing\n");
	retCode = pFrameWriter->write(*pFrameInstance,hdlFrameInstance[2-lType]);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to write Frame topic sample\n");
//	printf("Index: %d, LayerType: %d, Size: %d\n",(int)(pFrameInstance->index)
//												 ,(int)(pFrameInstance->layerType)
//												 ,(int)(pFrameInstance->payload.length()));
	pFrameInstance->index++;
}

NeuronVP::NeuronVP(const char *nameParam,const char *vidStats) : NeuronDP(nameParam,1,vidStats)
{
	startupVideoDataPublisher();
}

NeuronVP::~NeuronVP(void)
{
	DDS_ReturnCode_t	retCode;
	FrameDataWriter	   *pFrameWriter = NULL;

	for(int iPub=0; iPub<3; iPub++)
	{
		//Unregister instance of Frame topic
		pFrameWriter = FrameDataWriter::narrow(gpFrameWriter[iPub]);
		CHECK_HANDLE(pFrameWriter,"FrameDataWriter::narrow() error during shutdown\n");
		retCode = pFrameWriter->unregister_instance(*pFrameInstance,hdlFrameInstance[iPub]);
		CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Unable to unregister Frame instance\n");
	}

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

void NVPStartup(const char *name,const char *vidStats)
{
	pTheVideoPub = new NeuronVP(name,vidStats);
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
