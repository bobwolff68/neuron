#include "ECPSlaveObject.h"
#include "ECPSlave.h"

ECPSlaveObject::ECPSlaveObject(ECPSlave *_sl,int _sfId, int _sessionId,DDS_InstanceHandle_t stateH,DDS_InstanceHandle_t eventH,DDS_InstanceHandle_t metricsH) : ECPObject(_sfId,_sessionId)
{
    sl = _sl;
    epoch = 1;
    stateHandle = stateH;
    eventHandle = eventH;
    metricsHandle = metricsH;
};

ECPSlaveObject::~ECPSlaveObject()
{
}

bool ECPSlaveObject::Send(com::xvd::neuron::ecp::State *state)
{
    state->sessionId = sessionId;
    state->srcId = srcId;
    state->epoch = ++epoch;
    sl->Send(state,stateHandle);
    
    return true;
}

bool ECPSlaveObject::Send(com::xvd::neuron::ecp::Event *event)
{
    event->sessionId = sessionId;
    event->srcId = srcId;
    
    sl->Send(event,eventHandle);
    
    return true;
}

bool ECPSlaveObject::Send(com::xvd::neuron::ecp::Metrics *metrics)
{
    metrics->sessionId = sessionId;
    metrics->srcId = srcId;
    
    sl->Send(metrics,metricsHandle);
    
    return true;
}

DDS_InstanceHandle_t ECPSlaveObject::GetStateInstanceHandle()
{
    return stateHandle;
}

DDS_InstanceHandle_t ECPSlaveObject::GetEventInstanceHandle()
{
    return eventHandle;
}

DDS_InstanceHandle_t ECPSlaveObject::GetMetricsInstanceHandle()
{
    return metricsHandle;
}

