#include "ndds_cpp.h"
#include "ECPObject.h"
#include "ECPInterface.h"
#include "ECPInterfaceSupport.h"

ECPObject::ECPObject(int _srcId, int _sessionId) 
{
    sessionId = _sessionId;
    srcId = _srcId;
} 

ECPObject::~ECPObject() 
{
}

int ECPObject::GetSessionId() 
{
    return sessionId;
}

int ECPObject::GetSrcId() 
{
    return srcId;
}

