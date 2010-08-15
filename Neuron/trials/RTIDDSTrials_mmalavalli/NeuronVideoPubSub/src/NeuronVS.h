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
	char					curLayerChoice;
	DDSSubscriber		   *pVideoSub;			//Frame topic subscriber
	DDSDataReader		   *gpFrameReader;		//Generic pointer for Frame data reader
	DDSWaitSet			   *pWaitSet;			//Waitset for Frame samples
	DDSReadCondition	   *pWaitSetReadCond;	//Read condition for waitsets

	void	setupVDSMulticast			(void);
	void	changeVDSPartition(char layerChoice);
	void	setupWaitSet				(void);
	void	startupVideoDataSubscriber	(void);

	public:
		NeuronVS(const char *nameParam);
		~NeuronVS();

		void	setVDSPartition	(const char *partitionName);
		void	getFrame		(unsigned char **ppFrameBuf,int *pBufLen,char layerChoice);
};

NeuronVS	*pTheVideoSub = NULL;

#ifdef __cplusplus
extern "C"
{
#endif

//APIs
void	NVSStartup			(const char *name);
void	NVSSetVDSPartition	(const char *partitionName);
void	NVSGetFrame			(unsigned char **ppFrameBuf,int *pBufLen,char fpsChoice);
void	NVPDestroy			(void);

#ifdef __cplusplus
}
#endif

#endif /* NEURONVS_H_ */
