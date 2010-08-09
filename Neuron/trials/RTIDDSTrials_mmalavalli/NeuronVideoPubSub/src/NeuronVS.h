/*
 * NeuronVS.h
 *
 *  Created on: Aug 2, 2010
 *      Author: manjesh
 */

#ifndef NEURONVS_H_
#define NEURONVS_H_

#include "NeuronDP.h"

//Neuron video subscriber class
class NeuronVS : public NeuronDP
{
	DDSSubscriber		   *pVideoSub;			//Frame topic subscriber
	DDSPublisher		   *pThrotPub;			//ThrotMsg topic publisher
	DDSDataWriter		   *gpThrotWriter;		//Generic pointer for ThrotMsg data writer
	DDSDataReader		   *gpFrameReader;		//Generic pointer for Frame data reader
	DDSWaitSet			   *pWaitSet;			//Waitset for Frame samples

	void	setVDSPartition				(const char *partitionName);
	void	startupVideoDataSubscriber	(void);
	void	startupThrotMsgPublisher	(void);

	public:
		NeuronVS(const char *nameParam);
		~NeuronVS();

		void	setTMPPartition	(const char *partitionName);
		void	publishThrotMsg	(int modeVal);
		void	getFrame		(unsigned char **ppFrameBuf,int *pBufLen,char layerChoice);

};

NeuronVS	*pTheVideoSub = NULL;

#ifdef __cplusplus
extern "C"
{
#endif

//APIs
void	NVSStartup			(const char *name);
int		NVSGetVidSrcList	(const char ***pppVidSrcList);
void	NVSSetTMPPartition	(const char *partitionName);
void	NVSPublishThrotMsg	(int modeVal);
void	NVSGetFrame			(unsigned char **ppFrameBuf,int *pBufLen,char fpsChoice);
void	NVPDestroy			(void);

#ifdef __cplusplus
}
#endif

#endif /* NEURONVS_H_ */
