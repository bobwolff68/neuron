#include <stdio.h>
#include <stdlib.h>
#include "HelloWorld_publisher.h"

int main(int argc, char *argv[])
{
    int domainId = 0;
    int sample_count = 0; /* infinite loop */
	char* partitionName = "XXX";


    if (argc >= 2) {
        domainId = atoi(argv[1]);
    }
    if (argc >= 3) {
        sample_count = atoi(argv[2]);
    }
	if (argc >= 4) {
        partitionName = argv[3];
    }
    
    return publisher_main(domainId, sample_count, partitionName);
}

