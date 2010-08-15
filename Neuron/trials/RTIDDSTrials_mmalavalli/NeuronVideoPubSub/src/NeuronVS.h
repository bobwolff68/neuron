/*
 * NeuronVS.h
 *
 *  Created on: Aug 2, 2010
 *      Author: manjesh
 */

#ifndef NEURONVS_H_
#define NEURONVS_H_

#include "NeuronDP.h"

class FrameDataListener : public DDSDataReaderListener
{
    public:
        virtual void    on_data_available   (DDSDataReader *pGenericReader);
};

//Neuron video subscriber class
class NeuronVS : public NeuronDP
{
	DDSSubscriber		   *pVideoSub;			//Frame topic subscriber
	DDSDataReader		   *gpFrameReader;		//Generic pointer for Frame data reader

	void	setupVDSMulticast			(void);
	void	startupVideoDataSubscriber	(void);

	public:
		NeuronVS(const char *nameParam);
		~NeuronVS();

		void	setupFrameListener	(void);
		void	setVDSPartition		(const char *partitionName);
		void	changeVDSPartition	(char layerChoice);
};

NeuronVS	*pTheVideoSub = NULL;

#ifdef __cplusplus
extern "C"
{
#endif

//APIs
void	NVSStartup				(const char *name);
void	NVSSetupFrameListener	(void);
void	NVSSetVDSPartition		(const char *partitionName);
void	NVSChangeVDSPartition	(char fpsChoice);
void	NVPDestroy				(void);

#ifdef __cplusplus
}
#endif

#endif /* NEURONVS_H_ */
