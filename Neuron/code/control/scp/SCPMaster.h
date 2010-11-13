//!
//! \file SCPMaster.h
//!
//! \brief Defintion of Session Control Plane (SCP) Master
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#ifndef SCP_MASTER_H_
#define SCP_MASTER_H_

#include "SCPEvent.h"
#include "CPInterfaceT.h"
#include "SCPMasterObject.h"

//!
//! \class SCPMaster
//!
//! \brief SCP Master. This class attaches to the SCP plane as a master.
//!
//! Details: A SCP Master implements the following functions:
//!          - It is allowed to manage a scp through the Control interface
//!          - It tracks the state of a scp through the State, Event and
//!            Metrics interface.
//!          An application that wishes to create a new scp must create it
//!          using the CreateMasterObject. This object represents a scp in
//!          the SCP. Conversely, a scp must be deleted from the SCP using 
//!          the DeleteMasterObject.
//!
//! \todo GetMasterObjectNNN is not to be called directly.
//!
class SCPMaster : public CPMasterT<
SCPMasterObject,
com::xvd::neuron::scp::ControlDataWriter,
com::xvd::neuron::scp::StateDataReader,
com::xvd::neuron::scp::EventDataReader,
com::xvd::neuron::scp::MetricsDataReader,
com::xvd::neuron::scp::EventSeq,
com::xvd::neuron::scp::StateSeq,
com::xvd::neuron::scp::MetricsSeq,
com::xvd::neuron::scp::Metrics,
com::xvd::neuron::scp::Event,
com::xvd::neuron::scp::State,
com::xvd::neuron::scp::Control,
com::xvd::neuron::scp::ControlTypeSupport,
com::xvd::neuron::scp::EventTypeSupport,
com::xvd::neuron::scp::StateTypeSupport,
com::xvd::neuron::scp::MetricsTypeSupport> 
{
public:
    
    //! Constructor for the SCPMaster object
    //!
    //! \param[in] eh          Event-handler for all events received
    //! \param[in] srcId       Unique ID for the SCPMaster in the SCP
    //! \param[in] qosProfile  RTI DDS QoS profile to use
    SCPMaster(EventHandler *eh,int srcId, int domainId, const char *qosProfile);
    
    //! Desstructor for the SCPMaster object
    //!
    ~SCPMaster();
    
    virtual SCPMasterObject* CreateMasterObject(int sid);
    
    virtual bool DeleteMasterObject(SCPMasterObject* aSession);
    
    virtual DDS_InstanceHandle_t GetMasterObjectStateHandle(int dstId,int sid);
    
    virtual DDS_InstanceHandle_t GetMasterObjectEventHandle(int dstId, int sid);
    
    virtual DDS_InstanceHandle_t GetMasterObjectMetricsHandle(int dstId,int sid);    
};

template<class DataSeq, class Reader,class EventKind>
class SCPMasterReaderListenerT : public CPDataReaderListener
{
public:
    SCPMasterReaderListenerT(SCPMaster *_sm,Reader *reader) : CPDataReaderListener(_sm)
    {
        m_reader = reader;
        sm = _sm;
    }
    
    void on_data_available(DDSDataReader* reader)
    {
        DataSeq data_seq;
        DDS_SampleInfoSeq info_seq;
        DDS_ReturnCode_t retcode;
        int i;
        Event *ev;
        
        retcode = m_reader->read(data_seq, 
                                 info_seq, 
                                 DDS_LENGTH_UNLIMITED,
                                 DDS_NOT_READ_SAMPLE_STATE, 
                                 DDS_ANY_VIEW_STATE,
                                 DDS_ANY_INSTANCE_STATE);
        
        if (retcode == DDS_RETCODE_NO_DATA) {
            // TODO: Error logging
            return;
        } else if (retcode != DDS_RETCODE_OK) {
            // TODO: Error logging
            return;
        }
        
        for (i = 0; i < data_seq.length(); ++i) {
            switch (info_seq[i].view_state) {
                case DDS_NEW_VIEW_STATE:
                    if (!info_seq[i].valid_data) {
                        // TODO: Error logging
                    }   
                    break;
                case DDS_NOT_NEW_VIEW_STATE:
                    if (!info_seq[i].valid_data) {
                        // TODO: Error logging
                    } else {
                    }
                    break;
                default:
                    break;
            }
            
            switch (info_seq[i].instance_state) {
                case DDS_ALIVE_INSTANCE_STATE:
                    if (info_seq[i].valid_data) {
                        // TODO: Error logging
                    }
                    break;
                case DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE:
                    if (info_seq[i].valid_data) {
                        // TODO 
                    }
                    break;
                case DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE:
                    if (info_seq[i].valid_data) {
                        // TODO: Error logging
                    }
                    break;
                default:
                    break;
            }
            
            if (info_seq[i].valid_data) 
            {
                ev = new EventKind(&data_seq[i]);
                sm->PostEvent(ev);
            }
        }
        
        retcode = m_reader->return_loan(data_seq, info_seq);
        if (retcode != DDS_RETCODE_OK) {
            // TODO: Error logging
        }
    };
        
private:
    Reader *m_reader;
    SCPMaster *sm;    
};

typedef SCPMasterReaderListenerT<com::xvd::neuron::scp::StateSeq,
com::xvd::neuron::scp::StateDataReader,
SCPEventSessionStateUpdate> SCPMasterStateReaderListener;

typedef SCPMasterReaderListenerT<com::xvd::neuron::scp::EventSeq,
com::xvd::neuron::scp::EventDataReader,
SCPEventSessionEvent> SCPMasterEventReaderListener;

typedef SCPMasterReaderListenerT<com::xvd::neuron::scp::MetricsSeq,
com::xvd::neuron::scp::MetricsDataReader,
SCPEventSessionMetricsUpdate> SCPMasterMetricsReaderListener;

#endif
