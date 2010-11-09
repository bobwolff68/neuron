#include "SCPMasterObject.h"
#include "SCPMaster.h"

SCPMasterObject::SCPMasterObject(SCPMaster *_sm,int _sfId,int _sessionId,DDS_InstanceHandle_t _controlH) : SCPObject(_sfId,_sessionId)
{
    sm = _sm;
    controlHandle = _controlH;
};

bool SCPMasterObject::Send(com::xvd::neuron::session::Control* _c, int dstId)
{
    _c->sessionId = sessionId;
    _c->srcId = srcId;
    _c->dstId = dstId;
    return sm->Send(_c);
}

com::xvd::neuron::session::State* SCPMasterObject::GetState(void)
{
    sm->GetMasterObjectState(stateHandle,this->state);
    return this->state;
}

com::xvd::neuron::session::EventSeq* SCPMasterObject::GetEvents(void)
{
    sm->GetMasterObjectEvents(eventHandle,&this->eventSeq);
    return &this->eventSeq;
}

com::xvd::neuron::session::MetricsSeq* SCPMasterObject::GetMetrics(void)
{
    sm->GetMasterObjectMetrics(metricsHandle,&this->metricsSeq);
    return &this->metricsSeq;
}

DDS_InstanceHandle_t SCPMasterObject::GetControlInstanceHandle()
{
    return controlHandle;
}
