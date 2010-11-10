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

class SCPSlaveControlReaderListener : public CPDataReaderListener
{
    
public:
    SCPSlaveControlReaderListener(SCPSlave *_sl,
                                  com::xvd::neuron::session::ControlDataReader *reader);
    
    virtual void on_data_available(DDSDataReader* reader);
    
private:
    com::xvd::neuron::session::ControlDataReader *m_reader;
    SCPSlave *sl;
};

class SCPSlave : public CPSlaveT<com::xvd::neuron::session::ControlTypeSupport,
                                 com::xvd::neuron::session::EventTypeSupport,
                                 com::xvd::neuron::session::StateTypeSupport,
                                 com::xvd::neuron::session::MetricsTypeSupport> 
{
public:
    SCPSlave(EventHandler *q,int _sfId,int domainId,const char *qosProfile);
    
    ~SCPSlave();
    
    SCPSlaveObject* CreateSlaveObject(int sid);

    bool DeleteSlaveObject(SCPSlaveObject* aSession);
        
    bool Send(com::xvd::neuron::session::State *state, DDS_InstanceHandle_t ih);
    
    bool Send(com::xvd::neuron::session::Event *event, DDS_InstanceHandle_t ih);
    
    bool Send(com::xvd::neuron::session::Metrics *metrics, DDS_InstanceHandle_t ih);
    
    virtual bool PostEvent(Event *ev);
    
private:
    int srcId;
    EventHandler *upper;
    com::xvd::neuron::session::ControlDataReader *controlReader;
    com::xvd::neuron::session::StateDataWriter *stateWriter;
    com::xvd::neuron::session::EventDataWriter *eventWriter;
    com::xvd::neuron::session::MetricsDataWriter *metricsWriter;    
};

#endif
