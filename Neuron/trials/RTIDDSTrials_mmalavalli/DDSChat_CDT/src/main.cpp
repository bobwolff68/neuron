#include <stdio.h>
#include "DDSChat.h"
#include "DDSChatModule.h"

// Task 1: Detect newly online chat modules [DONE].
// Task 2: Detect users who go offline.
// Task 3: Publish/Subscribe chat messages [DONE w Partitions instead of Content Filtered Topics].

int main(int argc,char *argv[])
{
    if(argc==2)
    {
        DDSChatModule *pChatModule = new DDSChatModule(argv[1]);
        delete pChatModule;
    }
    else
        printf("Usage: <PathToExecutable>/DDSChat <Name>\n");

    return 0;
}

