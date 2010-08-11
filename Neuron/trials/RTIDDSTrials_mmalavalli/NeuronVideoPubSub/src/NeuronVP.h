/*
 * NeuronVP.h
 *
 *  Created on: Jul 30, 2010
 *      Author: manjesh
 */

#ifndef NEURONVP_H_
#define NEURONVP_H_

#include "NeuronDP.h"

//Neuron video publisher class
class NeuronVP : public NeuronDP
{
	DDSPublisher		   *pVideoPub;			//Frame topic publisher
	DDSDataWriter		   *gpFrameWriter;		//Generic pointer for Frame data writer
	DDS_InstanceHandle_t	hdlFrameInstance;
	Frame				   *pFrameInstance;

	void	startupVideoDataPublisher	(void);
	void	setVDPPartition				(const char *partitionName);

	public:
		NeuronVP(const char *nameParam,const char *vidStats);
		~NeuronVP();

		void	publishFrame	(unsigned char *pFrameBuf,int bufLen,int lType);
};

NeuronVP   *pTheVideoPub = NULL;

#ifdef __cplusplus
extern "C"
{
#endif

//APIs
void	NVPStartup			(const char *name,const char *vidStats);
void	NVPPublishFrame		(unsigned char *pFrameBuf,int bufLen,int lType);
void	NVPDestroy			(void);

#ifdef __cplusplus
}
#endif

#endif /* NEURONVP_H_ */
