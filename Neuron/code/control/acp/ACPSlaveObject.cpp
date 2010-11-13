#include "ACPSlaveObject.h"
#include "ACPSlave.h"

ACPSlaveObject::ACPSlaveObject(ACPSlave *_sl,int _sfId, int _sessionId,DDS_InstanceHandle_t stateH,DDS_InstanceHandle_t eventH,DDS_InstanceHandle_t metricsH) : ACPObject(_sfId,_sessionId)
{
    sl = _sl;
    epoch = 1;
    stateHandle = stateH;
    eventHandle = eventH;
    metricsHandle = metricsH;
};

ACPSlaveObject::~ACPSlaveObject()
{
}

bool ACPSlaveObject::Send(com::xvd::neuron::acp::State *state)
{
    state->srcId = srcId;
    state->epoch = ++epoch;
    sl->Send(state,stateHandle);
    
    return true;
}

bool ACPSlaveObject::Send(com::xvd::neuron::acp::Event *event)
{
    event->srcId = srcId;
    
    sl->Send(event,eventHandle);
    
    return true;
}

bool ACPSlaveObject::Send(com::xvd::neuron::acp::Metrics *metrics)
{
    metrics->srcId = srcId;
    
    sl->Send(metrics,metricsHandle);
    
    return true;
}

DDS_InstanceHandle_t ACPSlaveObject::GetStateInstanceHandle()
{
    return stateHandle;
}

DDS_InstanceHandle_t ACPSlaveObject::GetEventInstanceHandle()
{
    return eventHandle;
}

DDS_InstanceHandle_t ACPSlaveObject::GetMetricsInstanceHandle()
{
    return metricsHandle;
}

