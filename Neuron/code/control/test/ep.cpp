#include <stdio.h>
#include <stdlib.h>
#include "ndds_cpp.h"
#include "controlplane.h"

class MANE {
    
public:
    MANE(int appId,int domainId,const char *kind)
    {
        m_appId = appId;
        m_domainId = domainId;
        kind = strdup(kind);
    }
    
    ~MANE()
    {
    }
    
    void PrintInfo()
    {
        printf("MANE[%04d]: domaindId = %d, %s\n",m_appId,m_domainId,kind);
    }
    
private:
    int m_appId;
    int m_domainId;
    char *kind;
};

