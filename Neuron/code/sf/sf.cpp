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
	//printf("\n%s,%s\n",argv[1],argv[2]);
	SessionFactory	sf(sfId,argv[2],ownerId,domId);
	return 0;
}

