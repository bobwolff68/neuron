//!
//! \file LSCPSlaveObject.h
//!
//! \brief Defintion of Session Control Plane (LSCP) Slave Object
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#ifndef LSCP_SLAVE_OBJECT_H_
#define LSCP_SLAVE_OBJECT_H_

#include "LSCPObject.h"

class LSCPSlave;
class LSCPSlaveObject : public LSCPObject {
public:
    
    LSCPSlaveObject(LSCPSlave *_sl,int _sfId, int _sessionId,DDS_InstanceHandle_t,DDS_InstanceHandle_t,DDS_InstanceHandle_t);
    
    ~LSCPSlaveObject();
    
    bool Send(com::xvd::neuron::lscp::State *state);
    
    bool Send(com::xvd::neuron::lscp::Event *event);
    
    bool Send(com::xvd::neuron::lscp::Metrics *metrics);
    
    DDS_InstanceHandle_t GetStateInstanceHandle();

    DDS_InstanceHandle_t GetEventInstanceHandle();

    DDS_InstanceHandle_t GetMetricsInstanceHandle();

protected:
    LSCPSlave *sl;

    DDS_InstanceHandle_t stateHandle;
    DDS_InstanceHandle_t eventHandle;
    DDS_InstanceHandle_t metricsHandle;
    
    int epoch;
};

#endif
