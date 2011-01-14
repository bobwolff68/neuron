//!
//! \file ACPSlave.h
//!
//! \brief Defintion of Session Control Plane (ACP) Slave
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#ifndef ACP_SLAVE_H_
#define ACP_SLAVE_H_

#include "ACPEvent.h"
#include "CPInterfaceT.h"
#include "ACPSlaveObject.h"

class ACPSlaveControlReaderListener : public CPDataReaderListener
{
    
public:
    ACPSlaveControlReaderListener(ACPSlave *_sl,
                                  com::xvd::neuron::acp::ControlDataReader *reader);
    
    virtual void on_data_available(DDSDataReader* reader);
    
private:
    com::xvd::neuron::acp::ControlDataReader *m_reader;
    ACPSlave *sl;
};

class ACPSlave : public CPSlaveT<ACPSlaveObject,
                                 com::xvd::neuron::acp::ControlDataReader,
                                 com::xvd::neuron::acp::StateDataWriter,
                                 com::xvd::neuron::acp::EventDataWriter,
                                 com::xvd::neuron::acp::MetricsDataWriter,
                                 com::xvd::neuron::acp::State,
                                 com::xvd::neuron::acp::Event,
                                 com::xvd::neuron::acp::Metrics,
                                 com::xvd::neuron::acp::ControlTypeSupport,
                                 com::xvd::neuron::acp::EventTypeSupport,
                                 com::xvd::neuron::acp::StateTypeSupport,
                                 com::xvd::neuron::acp::MetricsTypeSupport> 
{
public:
    ACPSlave(EventHandler *q,int _sfId,int domainId,const char *name,const char *qosProfile);
    
    ~ACPSlave();
    
    virtual ACPSlaveObject* CreateSlaveObject(int sid);

    virtual bool DeleteSlaveObject(ACPSlaveObject* aSession);      
    
private:
    
    ACPSlaveControlReaderListener *controlListener;
};

#endif
