//!
//! \file ACPSlaveObject.cpp
//!
//! \brief Defintion of ACP Slave Object
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
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
    return sl->Send(state,stateHandle);
}

bool ACPSlaveObject::Send(com::xvd::neuron::acp::Event *event)
{
    event->srcId = srcId;
    
    return sl->Send(event,eventHandle);
}

bool ACPSlaveObject::Send(com::xvd::neuron::acp::Metrics *metrics)
{
    metrics->srcId = srcId;
    
    return sl->Send(metrics,metricsHandle);
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

