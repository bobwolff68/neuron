#include <stdio.h>
#include "sessionfactory.h"

int main(int argc, char *argv[])
{
	IDType 	sfId;
	int		domId;
	
	sscanf(argv[1],"%lld",&sfId);
	sscanf(argv[2],"%d",&domId);
	printf("\n%s,%s\n",argv[1],argv[2]);
	SessionFactory	sf(sfId,domId);
	printf("DONE");
	return 0;
}

