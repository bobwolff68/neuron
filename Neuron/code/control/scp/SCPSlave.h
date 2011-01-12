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
                                  com::xvd::neuron::scp::ControlDataReader *reader);

    virtual void on_data_available(DDSDataReader* reader);

private:
    com::xvd::neuron::scp::ControlDataReader *m_reader;
    SCPSlave *sl;
};

class SCPSlave : public CPSlaveT<SCPSlaveObject,
                                 com::xvd::neuron::scp::ControlDataReader,
                                 com::xvd::neuron::scp::StateDataWriter,
                                 com::xvd::neuron::scp::EventDataWriter,
                                 com::xvd::neuron::scp::MetricsDataWriter,
                                 com::xvd::neuron::scp::State,
                                 com::xvd::neuron::scp::Event,
                                 com::xvd::neuron::scp::Metrics,
                                 com::xvd::neuron::scp::ControlTypeSupport,
                                 com::xvd::neuron::scp::EventTypeSupport,
                                 com::xvd::neuron::scp::StateTypeSupport,
                                 com::xvd::neuron::scp::MetricsTypeSupport>
{
public:
    SCPSlave(EventHandler *q,int _sfId,int domainId,const char *name,const char *qosProfile);

    ~SCPSlave();

    virtual SCPSlaveObject* CreateSlaveObject(int sid);

    virtual bool DeleteSlaveObject(SCPSlaveObject* aSession);

    void StartupTwo(void);

private:

    SCPSlaveControlReaderListener *controlListener;
};

#endif
