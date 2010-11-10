#include "SCPSlaveObject.h"
#include "SCPSlave.h"

SCPSlaveObject::SCPSlaveObject(SCPSlave *_sl,int _sfId, int _sessionId,DDS_InstanceHandle_t stateH,DDS_InstanceHandle_t eventH,DDS_InstanceHandle_t metricsH) : SCPObject(_sfId,_sessionId)
{
    sl = _sl;
    stateHandle = stateH;
    eventHandle = eventH;
    metricsHandle = metricsH;
};

SCPSlaveObject::~SCPSlaveObject()
{
}

bool SCPSlaveObject::Send(com::xvd::neuron::session::State *state)
{
    state->sessionId = sessionId;
    state->srcId = srcId;
    
    sl->Send(state,stateHandle);
    
    return true;
}

bool SCPSlaveObject::Send(com::xvd::neuron::session::Event *event)
{
    event->sessionId = sessionId;
    event->srcId = srcId;
    
    sl->Send(event,eventHandle);
    
    return true;
}

bool SCPSlaveObject::Send(com::xvd::neuron::session::Metrics *metrics)
{
    metrics->sessionId = sessionId;
    metrics->srcId = srcId;
    
    sl->Send(metrics,metricsHandle);
    
    return true;
}

DDS_InstanceHandle_t SCPSlaveObject::GetStateInstanceHandle()
{
    return stateHandle;
}

DDS_InstanceHandle_t SCPSlaveObject::GetEventInstanceHandle()
{
    return eventHandle;
}

DDS_InstanceHandle_t SCPSlaveObject::GetMetricsInstanceHandle()
{
    return metricsHandle;
}

