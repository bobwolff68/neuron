#include <stdio.h>
#include "registration.h"
#include "sessionfactory.h"

int main(int argc, char *argv[])
{
	IDType 	            sfId;
	IDType	            ownerId;
	int		            domId;
	RegistrationClient *pRegClient = NULL;
	bool bUseAsEndpoint;

	bUseAsEndpoint = false;

	if (argc==7)
		bUseAsEndpoint=true;

	sscanf(argv[1],"%lld",&sfId);
	sscanf(argv[3],"%lld",&ownerId);
	sscanf(argv[4],"%d",&domId);
	pRegClient = new RegistrationClient(argv[5],sfId,8181,bUseAsEndpoint,argv[2]);
	pRegClient->registerClient();

	cout << "Found UBrain: (ACPMasterWanDesc: " << pRegClient->publicPairs["ubrain_acp_desc"]
	     << ", SCPMasterWanDesc: " << pRegClient->publicPairs["ubrain_scp_desc"] << endl;
	cout << "ACPSlaveWanId: " << pRegClient->publicPairs["client_acp_id"]
             << ", SCPSlaveWanId: " << pRegClient->publicPairs["client_scp_id"] << endl;

	if (bUseAsEndpoint)
	{
		sfId = FromStringNoChecking<int>(pRegClient->publicPairs["ep_sf_id"]);
		cout << "Using sf as an appliance with assigned ep_sf_id=" << sfId << endl;
	}
    if(pRegClient->publicPairs["use_lan_only"]=="true")
    {
        cout << "In LAN only mode..." << endl;
    }
    
	SessionFactory	*pSF = new SessionFactory(sfId,argv[2],ownerId,domId,pRegClient->publicPairs);
	
	pSF->startThread();
	while(!pSF->stop)
	{
		usleep(10000000);
	}
	pSF->stopThread();
	
	delete pSF;
	delete pRegClient;
	return 0;
}
