//!
//! \file SCPSlaveObject.h
//!
//! \brief Defintion of Session Control Plane (SCP) Slave Object
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#ifndef SCP_SLAVE_OBJECT_H_
#define SCP_SLAVE_OBJECT_H_

#include "SCPObject.h"

class SCPSlave;
class SCPSlaveObject : public SCPObject {
public:
    
    SCPSlaveObject(SCPSlave *_sl,int _sfId, int _sessionId,DDS_InstanceHandle_t,DDS_InstanceHandle_t,DDS_InstanceHandle_t);
    
    ~SCPSlaveObject();
    
    bool Send(com::xvd::neuron::session::State *state);
    
    bool Send(com::xvd::neuron::session::Event *event);
    
    bool Send(com::xvd::neuron::session::Metrics *metrics);
    
    DDS_InstanceHandle_t GetStateInstanceHandle();

    DDS_InstanceHandle_t GetEventInstanceHandle();

    DDS_InstanceHandle_t GetMetricsInstanceHandle();

protected:
    SCPSlave *sl;

    DDS_InstanceHandle_t stateHandle;
    DDS_InstanceHandle_t eventHandle;
    DDS_InstanceHandle_t metricsHandle;
};

#endif
