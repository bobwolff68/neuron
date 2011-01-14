//!
//! \file ACPSlaveObject.h
//!
//! \brief Defintion of Session Control Plane (ACP) Slave Object
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#ifndef ACP_SLAVE_OBJECT_H_
#define ACP_SLAVE_OBJECT_H_

#include "ACPObject.h"

class ACPSlave;
class ACPSlaveObject : public ACPObject {
public:
    
    ACPSlaveObject(ACPSlave *_sl,int _sfId, int _sessionId,DDS_InstanceHandle_t,DDS_InstanceHandle_t,DDS_InstanceHandle_t);
    
    ~ACPSlaveObject();
    
    bool Send(com::xvd::neuron::acp::State *state);
    
    bool Send(com::xvd::neuron::acp::Event *event);
    
    bool Send(com::xvd::neuron::acp::Metrics *metrics);
    
    DDS_InstanceHandle_t GetStateInstanceHandle();

    DDS_InstanceHandle_t GetEventInstanceHandle();

    DDS_InstanceHandle_t GetMetricsInstanceHandle();

protected:
    ACPSlave *sl;

    DDS_InstanceHandle_t stateHandle;
    DDS_InstanceHandle_t eventHandle;
    DDS_InstanceHandle_t metricsHandle;
    
    int epoch;
};

#endif
