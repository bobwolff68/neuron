#include "ndds_cpp.h"
#include "SCPObject.h"
#include "SCPInterface.h"
#include "SCPInterfaceSupport.h"

SCPObject::SCPObject(int _srcId, int _sessionId) 
{
    sessionId = _sessionId;
    srcId = _srcId;
} 

SCPObject::~SCPObject() 
{
}

int SCPObject::GetSessionId() 
{
    return sessionId;
}

int SCPObject::GetSrcId() 
{
    return srcId;
}

