//!
//! \file ECPSlaveObject.h
//!
//! \brief Defintion of Session Control Plane (ECP) Slave Object
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#ifndef ECP_SLAVE_OBJECT_H_
#define ECP_SLAVE_OBJECT_H_

#include "ECPObject.h"

class ECPSlave;
class ECPSlaveObject : public ECPObject {
public:
    
    ECPSlaveObject(ECPSlave *_sl,int _sfId, int _sessionId,DDS_InstanceHandle_t,DDS_InstanceHandle_t,DDS_InstanceHandle_t);
    
    ~ECPSlaveObject();
    
    bool Send(com::xvd::neuron::ecp::State *state);
    
    bool Send(com::xvd::neuron::ecp::Event *event);
    
    bool Send(com::xvd::neuron::ecp::Metrics *metrics);
    
    DDS_InstanceHandle_t GetStateInstanceHandle();

    DDS_InstanceHandle_t GetEventInstanceHandle();

    DDS_InstanceHandle_t GetMetricsInstanceHandle();

protected:
    ECPSlave *sl;

    DDS_InstanceHandle_t stateHandle;
    DDS_InstanceHandle_t eventHandle;
    DDS_InstanceHandle_t metricsHandle;
    
    int epoch;
};

#endif
