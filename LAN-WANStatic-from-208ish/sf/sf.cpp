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
	pRegClient = new RegistrationClient(argv[5],8181,false,argv[2]);
	pRegClient->registerClient();
	SessionFactory	sf(sfId,argv[2],ownerId,domId,pRegClient->publicPairs["client_scp_id"].c_str(),
                       pRegClient->publicPairs["client_acp_id"].c_str(),pRegClient->publicPairs["ubrain_scp_desc"].c_str(),
                       pRegClient->publicPairs["ubrain_acp_desc"].c_str());
	sf.startThread();
	while(!sf.stop)
	{
		usleep(20000);
	}
	sf.stopThread();
	return 0;
}
