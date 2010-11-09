#include "ndds_cpp.h"
#include "SCPObject.h"
#include "SCPInterface.h"
#include "SCPInterfaceSupport.h"

SCPObject::SCPObject(int _srcId, int _sessionId) 
{
    sessionId = _sessionId;
    srcId = _srcId;
    control = com::xvd::neuron::session::ControlTypeSupport::create_data();
    control->sessionId = _sessionId;
};

SCPObject::~SCPObject() 
{
    com::xvd::neuron::session::ControlTypeSupport::delete_data(control);
};

int SCPObject::GetSessionId() 
{
    return sessionId;
}

int SCPObject::GetSrcId() 
{
    return srcId;
}

