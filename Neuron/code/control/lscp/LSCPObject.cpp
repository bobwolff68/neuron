#include "ndds_cpp.h"
#include "LSCPObject.h"
#include "LSCPInterface.h"
#include "LSCPInterfaceSupport.h"

LSCPObject::LSCPObject(int _srcId, int _sessionId) 
{
    sessionId = _sessionId;
    srcId = _srcId;
} 

LSCPObject::~LSCPObject() 
{
}

int LSCPObject::GetSessionId() 
{
    return sessionId;
}

int LSCPObject::GetSrcId() 
{
    return srcId;
}

