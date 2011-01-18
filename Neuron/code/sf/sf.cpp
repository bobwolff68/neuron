#include <stdio.h>
#include "registration.h"
#include "sessionfactory.h"

int main(int argc, char *argv[])
{
	IDType 	            sfId;
	IDType	            ownerId;
	int		            domId;
	RegistrationClient *pRegClient = NULL;

	sscanf(argv[1],"%lld",&sfId);
	sscanf(argv[3],"%lld",&ownerId);
	sscanf(argv[4],"%d",&domId);
	pRegClient = new RegistrationClient(argv[5],sfId,8181,false,argv[2]);
	pRegClient->registerClient();

	cout << "Found UBrain: (ACPMasterWanDesc: " << pRegClient->publicPairs["ubrain_acp_desc"]
	     << ", SCPMasterWanDesc: " << pRegClient->publicPairs["ubrain_scp_desc"] << endl;
	cout << "ACPSlaveWanId: " << pRegClient->publicPairs["client_acp_id"]
             << ", SCPSlaveWanId: " << pRegClient->publicPairs["client_scp_id"] << endl;

	SessionFactory	sf(sfId,argv[2],ownerId,domId,pRegClient->publicPairs);
	sf.startThread();
	while(!sf.stop)
	{
		usleep(20000);
	}
	sf.stopThread();
	
	delete pRegClient;
	return 0;
}
