#include <stdio.h>
#include <unistd.h>
#include "DDSChat.h"
#include "DDSChatModule.h"

#define SLEEP_TIME_MUS  100000000

// Task 1: Detect newly online chat modules [DONE].
// Task 2: Detect users who go offline.
// Task 3: Publish/Subscribe chat messages.

int main(int argc,char *argv[])
{
    long    userID;

    if(argc==3)
    {
        sscanf(argv[2],"%ld",&userID);
        DDSChatModule *pChatModule = new DDSChatModule(userID,argv[1]);
        pChatModule->startup();

        // Sleep for SLEEP_TIME_MUS microseconds to detect
        // other users who come online
        usleep(SLEEP_TIME_MUS);

        delete pChatModule;
    }
    else
        printf("Usage: [<PathToExecutable>/]DDSChat <Name> <ID>\n");

    return 0;
}

