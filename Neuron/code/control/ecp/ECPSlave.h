//!
//! \file ECPSlave.h
//!
//! \brief Defintion of Session Control Plane (ECP) Slave
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#ifndef ECP_SLAVE_H_
#define ECP_SLAVE_H_

#include "ECPEvent.h"
#include "CPInterfaceT.h"
#include "ECPSlaveObject.h"

class ECPSlaveControlReaderListener : public CPDataReaderListener
{
    
public:
    ECPSlaveControlReaderListener(ECPSlave *_sl,
                                  com::xvd::neuron::ecp::ControlDataReader *reader);
    
    virtual void on_data_available(DDSDataReader* reader);
    
private:
    com::xvd::neuron::ecp::ControlDataReader *m_reader;
    ECPSlave *sl;
};

class ECPSlave : public CPSlaveT<ECPSlaveObject,
                                 com::xvd::neuron::ecp::ControlDataReader,
                                 com::xvd::neuron::ecp::StateDataWriter,
                                 com::xvd::neuron::ecp::EventDataWriter,
                                 com::xvd::neuron::ecp::MetricsDataWriter,
                                 com::xvd::neuron::ecp::State,
                                 com::xvd::neuron::ecp::Event,
                                 com::xvd::neuron::ecp::Metrics,
                                 com::xvd::neuron::ecp::ControlTypeSupport,
                                 com::xvd::neuron::ecp::EventTypeSupport,
                                 com::xvd::neuron::ecp::StateTypeSupport,
                                 com::xvd::neuron::ecp::MetricsTypeSupport> 
{
public:
    ECPSlave(EventHandler *q,int _sfId,int domainId,const char *qosProfile);
    
    ~ECPSlave();
    
    virtual ECPSlaveObject* CreateSlaveObject(int sid);

    virtual bool DeleteSlaveObject(ECPSlaveObject* aSession);      
    
private:
    
    ECPSlaveControlReaderListener *controlListener;
};

#endif
