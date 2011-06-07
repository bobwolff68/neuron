#include <iostream>
#include "sessionfactoryprocess.h"

Q_DEFINE_THIS_FILE

using namespace SessionFactoryProcess;

int main(int argc, char *argv[])
{
    if(Init(argc,argv))
    {
        Run();
        Teardown();
    }
    
    return 0;
}
