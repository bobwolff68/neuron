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
	DDSPublisher		   *pVideoPub[3];			//Frame topic publishers for 3 temporal layers
	DDSDataWriter		   *gpFrameWriter[3];		//Generic pointers for Frame data writers for
													//3 temporal layers
	DDS_InstanceHandle_t	hdlFrameInstance[3];
	Frame				   *pFrameInstance;

	void	startupVideoDataPublisher	(void);
	void	setVDPPartition				(int pubIndex,const char *partitionName);

	public:
		NeuronVP(const char *nameParam,const char *vidStats);
		~NeuronVP();

		void	publishFrame	(unsigned char *pFrameBuf,int bufLen,int lType);
};

/* NOTE: The partition string has the format <srcName>/L0, <srcName>/L1, and <srcName>/L2.
 * Through experiments, it was determined that if the subscriber partition string
 * is <srcName>/L0, DDS will match it with all partitions starting from it in ascending
 * order. So, for our purposes, L0 frames are written in partition "<srcName>/L2",
 * L1 frames in "<srcName>/L1" and L2 frames in "<srcName>/L0".
 *
 * 1. Full frame rate: Subscriber partition = <srcName>/L0.
 * 2. Half frame rate: Subscriber partition = <srcName>/L1.
 * 3. Qrtr frame rate: Subscriber partition = <srcName>/L2.
 */

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
