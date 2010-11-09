//!
//! \file SCPSlave.h
//!
//! \brief Defintion of Session Control Plane (SCP) Slave
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#ifndef SCP_SLAVE_H_
#define SCP_SLAVE_H_

#include "SCPEvent.h"
#include "CPInterfaceT.h"
#include "SCPSlaveObject.h"

class SCPSlave : public CPSlaveT<com::xvd::neuron::session::ControlTypeSupport,
                                 com::xvd::neuron::session::EventTypeSupport,
                                 com::xvd::neuron::session::StateTypeSupport,
                                 com::xvd::neuron::session::MetricsTypeSupport> 
{
public:
    SCPSlave(EventHandler *q,int _sfId,int domainId,const char *qosProfile);
    
    SCPSlaveObject* CreateSlaveObject(int sid);

    bool DeleteSlaveObject(SCPSlaveObject* aSession);
        
    bool Send(com::xvd::neuron::session::State *state);
    
    bool Send(com::xvd::neuron::session::Event *event);
    
    bool Send(com::xvd::neuron::session::Metrics *metrics);
    
    bool PostEvent(Event *ev);
    
private:
    int srcId;
    EventHandler *upper;
    com::xvd::neuron::session::ControlDataReader *controlReader;
    com::xvd::neuron::session::StateDataWriter *stateWriter;
    com::xvd::neuron::session::EventDataWriter *eventWriter;
    com::xvd::neuron::session::MetricsDataWriter *metricsWriter;    
};
#endif
