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
    
    return sl->Send(state,stateHandle);
}

bool ECPSlaveObject::Send(com::xvd::neuron::ecp::Event *event)
{
    event->sessionId = sessionId;
    event->srcId = srcId;
    
    return sl->Send(event,eventHandle);
}

bool ECPSlaveObject::Send(com::xvd::neuron::ecp::Metrics *metrics)
{
    metrics->sessionId = sessionId;
    metrics->srcId = srcId;
    
    return sl->Send(metrics,metricsHandle);
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

