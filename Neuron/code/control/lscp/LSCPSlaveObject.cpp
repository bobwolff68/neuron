#include "LSCPSlaveObject.h"
#include "LSCPSlave.h"

LSCPSlaveObject::LSCPSlaveObject(LSCPSlave *_sl,int _sfId, int _sessionId,DDS_InstanceHandle_t stateH,DDS_InstanceHandle_t eventH,DDS_InstanceHandle_t metricsH) : LSCPObject(_sfId,_sessionId)
{
    sl = _sl;
    epoch = 1;
    stateHandle = stateH;
    eventHandle = eventH;
    metricsHandle = metricsH;
};

LSCPSlaveObject::~LSCPSlaveObject()
{
}

bool LSCPSlaveObject::Send(com::xvd::neuron::lscp::State *state)
{
    state->sessionId = sessionId;
    state->srcId = srcId;
    state->epoch = ++epoch;
    sl->Send(state,stateHandle);
    
    return true;
}

bool LSCPSlaveObject::Send(com::xvd::neuron::lscp::Event *event)
{
    event->sessionId = sessionId;
    event->srcId = srcId;
    
    sl->Send(event,eventHandle);
    
    return true;
}

bool LSCPSlaveObject::Send(com::xvd::neuron::lscp::Metrics *metrics)
{
    metrics->sessionId = sessionId;
    metrics->srcId = srcId;
    
    sl->Send(metrics,metricsHandle);
    
    return true;
}

DDS_InstanceHandle_t LSCPSlaveObject::GetStateInstanceHandle()
{
    return stateHandle;
}

DDS_InstanceHandle_t LSCPSlaveObject::GetEventInstanceHandle()
{
    return eventHandle;
}

DDS_InstanceHandle_t LSCPSlaveObject::GetMetricsInstanceHandle()
{
    return metricsHandle;
}

