/*
 * NeuronVP.h
 *
 *  Created on: Jul 30, 2010
 *      Author: manjesh
 */

#ifndef NEURONVP_H_
#define NEURONVP_H_

#include "NeuronDP.h"

class DDSThrotMsgListener : public DDSDataReaderListener
{
    public:
        virtual void    on_data_available   (DDSDataReader *pGenericReader);
};

//Neuron video publisher class
class NeuronVP : public NeuronDP
{
	DDSPublisher		   *pVideoPub;			//Frame topic publisher
	DDSSubscriber		   *pThrotSub;			//ThrotMsg topic subscriber
	DDSDataWriter		   *gpFrameWriter;		//Generic pointer for Frame data writer
	DDSDataReader		   *gpThrotReader;		//Generic pointer for ThrotMsg data reader
	DDS_InstanceHandle_t	hdlFrameInstance;
	Frame				   *pFrameInstance;

	void	setTMSPartition				(const char *partitionName);
	void	startupVideoDataPublisher	(void);
	void	startupThrotMsgSubscriber	(void);

	public:
		NeuronVP(const char *nameParam);
		~NeuronVP();

		void	setVDPPartition	(const char *partitionName);
		void	publishFrame	(unsigned char *pFrameBuf,int bufLen,int lType);
};

NeuronVP   *pTheVideoPub = NULL;
int		   *pTheThrotMode = NULL;
char		vidSinkName[MAX_NAME_LEN];

#ifdef __cplusplus
extern "C"
{
#endif

//APIs
void	NVPStartup			(const char *name);
void	NVPSetThrotModePtr	(int *pThrotMode);
void	NVPSetVDPPartition	(void);
void	NVPPublishFrame		(unsigned char *pFrameBuf,int bufLen,int lType);
void	NVPDestroy			(void);

#ifdef __cplusplus
}
#endif

#endif /* NEURONVP_H_ */
