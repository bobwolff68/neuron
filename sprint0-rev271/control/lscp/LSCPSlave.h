//!
//! \file LSCPSlave.h
//!
//! \brief Defintion of Session Control Plane (LSCP) Slave
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#ifndef LSCP_SLAVE_H_
#define LSCP_SLAVE_H_

#include "LSCPEvent.h"
#include "CPInterfaceT.h"
#include "LSCPSlaveObject.h"

class LSCPSlaveControlReaderListener : public CPDataReaderListener
{
    
public:
    LSCPSlaveControlReaderListener(LSCPSlave *_sl,
                                  com::xvd::neuron::lscp::ControlDataReader *reader);
    
    virtual void on_data_available(DDSDataReader* reader);
    
private:
    com::xvd::neuron::lscp::ControlDataReader *m_reader;
    LSCPSlave *sl;
};

class LSCPSlave : public CPSlaveT<LSCPSlaveObject,
                                 com::xvd::neuron::lscp::ControlDataReader,
                                 com::xvd::neuron::lscp::StateDataWriter,
                                 com::xvd::neuron::lscp::EventDataWriter,
                                 com::xvd::neuron::lscp::MetricsDataWriter,
                                 com::xvd::neuron::lscp::State,
                                 com::xvd::neuron::lscp::Event,
                                 com::xvd::neuron::lscp::Metrics,
                                 com::xvd::neuron::lscp::ControlTypeSupport,
                                 com::xvd::neuron::lscp::EventTypeSupport,
                                 com::xvd::neuron::lscp::StateTypeSupport,
                                 com::xvd::neuron::lscp::MetricsTypeSupport> 
{
public:
    LSCPSlave(EventHandler *q,int _sfId,int domainId,const char *,map<string,string> &PropertyPairs,
              map<string,DDS_Boolean> &PropagateDiscoveryFlags,const char *qosProfile);
    
    ~LSCPSlave();
    
    virtual LSCPSlaveObject* CreateSlaveObject(int sid);

    virtual bool DeleteSlaveObject(LSCPSlaveObject* aSession);      
    
private:
    
    LSCPSlaveControlReaderListener *controlListener;
};

#endif
