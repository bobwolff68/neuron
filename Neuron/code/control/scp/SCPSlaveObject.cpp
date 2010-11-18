//!
//! \file SCPMasterObject.cpp
//!
//! \brief Defintion of SCP Master Object
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#include "SCPSlaveObject.h"
#include "SCPSlave.h"

//! \brief Contstructor for a SCPMasterObject
//!
//! \param[in] sl         - slave object
//! \param[in] srcId      - source application id
//! \param[in] sessionId  - session id
//! \param[in] stateH     - state instance handle
//! \param[in] eventH     - event instance handle
//! \param[in] metricsH   - metrics instance handle
SCPSlaveObject::SCPSlaveObject(SCPSlave *_sl,int _sfId, int _sessionId,DDS_InstanceHandle_t stateH,DDS_InstanceHandle_t eventH,DDS_InstanceHandle_t metricsH) : SCPObject(_sfId,_sessionId)
{
    sl = _sl;
    epoch = 1;
    stateHandle = stateH;
    eventHandle = eventH;
    metricsHandle = metricsH;
};

SCPSlaveObject::~SCPSlaveObject()
{
}

//! \brief Send state data to a destination
//!
//! \return true on success, false on failure
bool SCPSlaveObject::Send(com::xvd::neuron::scp::State *state)
{
    state->sessionId = sessionId;
    state->srcId = srcId;
    state->epoch = ++epoch;
    
    return sl->Send(state,stateHandle);
}

//! \brief Send event data to a destination
//!
//! \return true on success, false on failure
bool SCPSlaveObject::Send(com::xvd::neuron::scp::Event *event)
{
    event->sessionId = sessionId;
    event->srcId = srcId;

    return sl->Send(event,eventHandle);
}

//! \brief Send metrics data to a destination
//!
//! \return true on success, false on failure
bool SCPSlaveObject::Send(com::xvd::neuron::scp::Metrics *metrics)
{
    metrics->sessionId = sessionId;
    metrics->srcId = srcId;
    
    return sl->Send(metrics,metricsHandle);
}

//! \brief Get state instance handle for object
//!
//! \return state instance handle
DDS_InstanceHandle_t SCPSlaveObject::GetStateInstanceHandle()
{
    return stateHandle;
}

//! \brief Get event instance handle for object
//!
//! \return event instance handle
DDS_InstanceHandle_t SCPSlaveObject::GetEventInstanceHandle()
{
    return eventHandle;
}

//! \brief Get metrics instance handle for object
//!
//! \return metrics instance handle
DDS_InstanceHandle_t SCPSlaveObject::GetMetricsInstanceHandle()
{
    return metricsHandle;
}

