#include "SCPMasterObject.h"
#include "SCPMaster.h"

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
    
    state = com::xvd::neuron::session::StateTypeSupport::create_data();    
};

SCPMasterObject::~SCPMasterObject()
{
    if (state != NULL)
    {
        com::xvd::neuron::session::StateTypeSupport::delete_data(state);
    }
}

bool SCPMasterObject::Send(com::xvd::neuron::session::Control* _c, int dstId)
{
    _c->sessionId = sessionId;
    _c->srcId = srcId;
    _c->dstId = dstId;
    return sm->Send(_c,controlHandle);
}

com::xvd::neuron::session::State* SCPMasterObject::GetState(void)
{
    
    if (state == NULL)
    {
        //TODO: Error logging
        return NULL;
    }
    
    sm->GetMasterObjectState(stateHandle,this->state);
    return this->state;
}

com::xvd::neuron::session::EventSeq* SCPMasterObject::GetEvents(void)
{
    //NOTE: Data is _not_ loaned from DDS
    sm->GetMasterObjectEvents(eventHandle,&this->eventSeq);
    return &this->eventSeq;
}

com::xvd::neuron::session::MetricsSeq* SCPMasterObject::GetMetrics(void)
{
    //NOTE: Data is _not_ loaned from DDS
    sm->GetMasterObjectMetrics(metricsHandle,&this->metricsSeq);
    return &this->metricsSeq;
}

DDS_InstanceHandle_t SCPMasterObject::GetControlInstanceHandle()
{
    return controlHandle;
}
