#include <stdio.h>
#include "sessionfactory.h"

int main(int argc, char *argv[])
{
	IDType 	sfId;
	IDType	ownerId;
	int		domId;
	
	
	sscanf(argv[1],"%lld",&sfId);
	sscanf(argv[3],"%lld",&ownerId);
	sscanf(argv[4],"%d",&domId);
	SessionFactory	sf(sfId,argv[2],ownerId,domId);
	sf.startThread();
	while(!sf.stop)
	{
		usleep(20000);
		//com::xvd::neuron::lscp::State *pState = com::xvd::neuron::lscp::StateTypeSupport::create_data();
		//pState->srcId = sfId;
		//pState->sessionId = 1001;
		//pState->state = com::xvd::neuron::OBJECT_STATE_OFFERSRC;
		//strcpy(pState->payload,"srcId,srcType,resW,resH");
		//sf.SignalEvent(new LSCPEventSessionStateUpdate(pState,new (DDS_SampleInfo)));
		//com::xvd::neuron::lscp::StateTypeSupport::delete_data(pState);
	}
	sf.stopThread();
	return 0;
}

