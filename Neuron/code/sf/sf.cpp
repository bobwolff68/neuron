#include <stdio.h>
#include "sessionfactory.h"

int main(int argc, char *argv[])
{
	IDType sfId;
	
	sscanf(argv[1],"%lld",&sfId);
	SessionFactory	sf(sfId);
	return 0;
}

