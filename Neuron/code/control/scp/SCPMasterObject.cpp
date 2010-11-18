//!
//! \file SCPMasterObject.cpp
//!
//! \brief Defintion of SCP Master Object
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#include "SCPMasterObject.h"
#include "SCPMaster.h"

//! \brief Contstructor for a SCPMasterObject
//!
//! \param[in] sm         - master object
//! \param[in] srcId      - source application id
//! \param[in] sessionId  - session id
//! \param[in] controlH   - control instance handle
//! \param[in] stateH     - state instance handle
//! \param[in] eventH     - event instance handle
//! \param[in] metricsH   - metrics instance handle
//!    
SCPMasterObject::SCPMasterObject(SCPMaster *_sm,
                                 int _sfId,
                                 int _sessionId,
                                 DDS_InstanceHandle_t _controlH,
                                 DDS_InstanceHandle_t _stateH,
                                 DDS_InstanceHandle_t _eventH,
                                 DDS_InstanceHandle_t _metricsH) : SCPObject(_sfId,_sessionId)
{
    sm = _sm;
    
    controlHandle = _controlH;
    metricsHandle = _metricsH;
    eventHandle = _eventH;
    stateHandle = _stateH;
    
    state = com::xvd::neuron::scp::StateTypeSupport::create_data();    
};

//! \brief Destructor for a SCPMasterObject
//
SCPMasterObject::~SCPMasterObject()
{
    if (state != NULL)
    {
        com::xvd::neuron::scp::StateTypeSupport::delete_data(state);
    }
}

//! \brief Send control data to a destination
//!
//! \return true on success, false on failure
bool SCPMasterObject::Send(com::xvd::neuron::scp::Control* _c, int dstId)
{
    _c->sessionId = sessionId;
    _c->srcId = srcId;
    _c->dstId = dstId;
    return sm->Send(_c,controlHandle);
}

//! \brief Get current state for a slave
//! \param [in] - slave application id
//!
//! \return state on success, NULL in failure
com::xvd::neuron::scp::State* SCPMasterObject::GetState(int dstId)
{
    
    if (state == NULL)
    {
        //TODO: Error logging
        ControlLogError("state == NULL");
        return NULL;
    }
    stateHandle = sm->GetMasterObjectStateHandle(dstId,sessionId);
    if (!DDS_InstanceHandle_is_nil(&stateHandle))
    {
        sm->GetMasterObjectState(stateHandle,this->state);
        return this->state;
    }
    return NULL;
}

//! \brief Get current events for a slave
//! \param [in] - slave application id
//!
//! \return sequence of last N events on success, NULL in failure
com::xvd::neuron::scp::EventSeq* SCPMasterObject::GetEvents(int dstId)
{
    //NOTE: Data is _not_ loaned from DDS
    this->eventSeq.length(0);
    this->eventSeq.maximum(0);
    
    eventHandle = sm->GetMasterObjectEventHandle(dstId,sessionId);
    if (!DDS_InstanceHandle_is_nil(&eventHandle))
    {   
        sm->GetMasterObjectEvents(eventHandle,&this->eventSeq);
        return &this->eventSeq;
    }
    return NULL;
}

//! \brief Get current metrics for a slave
//! \param [in] - slave application id
//!
//! \return sequence of last N metrics on success, NULL in failure
com::xvd::neuron::scp::MetricsSeq* SCPMasterObject::GetMetrics(int dstId)
{
    this->metricsSeq.length(0);
    this->metricsSeq.maximum(0);

    //NOTE: Data is _not_ loaned from DDS
    metricsHandle = sm->GetMasterObjectMetricsHandle(dstId,sessionId);
    if (!DDS_InstanceHandle_is_nil(&metricsHandle))
    {
        sm->GetMasterObjectMetrics(metricsHandle,&this->metricsSeq);
        return &this->metricsSeq;
    }
    return NULL;
}

//! \brief Get control instance handle for object
//!
//! \return control instance handle
DDS_InstanceHandle_t SCPMasterObject::GetControlInstanceHandle()
{
    return controlHandle;
}
