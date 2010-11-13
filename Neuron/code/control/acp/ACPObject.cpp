#include "ndds_cpp.h"
#include "ACPObject.h"
#include "ACPInterface.h"
#include "ACPInterfaceSupport.h"

ACPObject::ACPObject(int _srcId, int _sessionId) 
{
    sessionId = _sessionId;
    srcId = _srcId;
} 

ACPObject::~ACPObject() 
{
}

int ACPObject::GetSessionId() 
{
    return sessionId;
}

int ACPObject::GetSrcId() 
{
    return srcId;
}

