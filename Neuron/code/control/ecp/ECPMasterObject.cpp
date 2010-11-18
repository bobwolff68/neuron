//!
//! \file ECPMasterObject.cpp
//!
//! \brief Defintion of ECP Master Object
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#include "ECPMasterObject.h"
#include "ECPMaster.h"

ECPMasterObject::ECPMasterObject(ECPMaster *_sm,
                                 int _sfId,
                                 int _sessionId,
                                 DDS_InstanceHandle_t _controlH,
                                 DDS_InstanceHandle_t _stateH,
                                 DDS_InstanceHandle_t _eventH,
                                 DDS_InstanceHandle_t _metricsH) : ECPObject(_sfId,_sessionId)
{
    sm = _sm;
    
    controlHandle = _controlH;
    metricsHandle = _metricsH;
    eventHandle = _eventH;
    stateHandle = _stateH;
    
    state = com::xvd::neuron::ecp::StateTypeSupport::create_data();    
};

ECPMasterObject::~ECPMasterObject()
{
    if (state != NULL)
    {
        com::xvd::neuron::ecp::StateTypeSupport::delete_data(state);
    }
}

bool ECPMasterObject::Send(com::xvd::neuron::ecp::Control* _c, int dstId)
{
    _c->sessionId = sessionId;
    _c->srcId = srcId;
    _c->dstId = dstId;
    return sm->Send(_c,controlHandle);
}

com::xvd::neuron::ecp::State* ECPMasterObject::GetState(int dstId)
{
    
    if (state == NULL)
    {
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

com::xvd::neuron::ecp::EventSeq* ECPMasterObject::GetEvents(int dstId)
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

com::xvd::neuron::ecp::MetricsSeq* ECPMasterObject::GetMetrics(int dstId)
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

DDS_InstanceHandle_t ECPMasterObject::GetControlInstanceHandle()
{
    return controlHandle;
}
