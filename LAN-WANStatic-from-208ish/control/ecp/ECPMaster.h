//!
//! \file ECPMaster.h
//!
//! \brief Defintion of Session Control Plane (ECP) Master
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#ifndef ECP_MASTER_H_
#define ECP_MASTER_H_

#include "ECPEvent.h"
#include "CPInterfaceT.h"
#include "ECPMasterObject.h"

//!
//! \class ECPMaster
//!
//! \brief ECP Master. This class attaches to the ECP plane as a master.
//!
//! Details: A ECP Master implements the following functions:
//!          - It is allowed to manage a session through the Control interface
//!          - It tracks the state of a session through the State, Event and
//!            Metrics interface.
//!          An application that wishes to create a new session must create it
//!          using the CreateMasterObject. This object represents a session in
//!          the ECP. Conversely, a session must be deleted from the ECP using 
//!          the DeleteMasterObject.
//!
//! \todo GetMasterObjectNNN is not to be called directly.
//!
class ECPMaster : public CPMasterT<
ECPMasterObject,
com::xvd::neuron::ecp::ControlDataWriter,
com::xvd::neuron::ecp::StateDataReader,
com::xvd::neuron::ecp::EventDataReader,
com::xvd::neuron::ecp::MetricsDataReader,
com::xvd::neuron::ecp::EventSeq,
com::xvd::neuron::ecp::StateSeq,
com::xvd::neuron::ecp::MetricsSeq,
com::xvd::neuron::ecp::Metrics,
com::xvd::neuron::ecp::Event,
com::xvd::neuron::ecp::State,
com::xvd::neuron::ecp::Control,
com::xvd::neuron::ecp::ControlTypeSupport,
com::xvd::neuron::ecp::EventTypeSupport,
com::xvd::neuron::ecp::StateTypeSupport,
com::xvd::neuron::ecp::MetricsTypeSupport> 
{
public:
    
    //! Constructor for the ECPMaster object
    //!
    //! \param[in] eh          Event-handler for all events received
    //! \param[in] srcId       Unique ID for the ECPMaster in the ECP
    //! \param[in] qosProfile  RTI DDS QoS profile to use
    ECPMaster(EventHandler *eh,int srcId, int domainId, const char *name,const char *qosProfile);
    
    //! Desstructor for the ECPMaster object
    //!
    ~ECPMaster();
    
    virtual ECPMasterObject* CreateMasterObject(int sid);
    
    virtual bool DeleteMasterObject(ECPMasterObject* aSession);
};

template<class DataSeq, class Reader,class EventKind>
class ECPMasterReaderListenerT : public CPDataReaderListener
{
public:
    ECPMasterReaderListenerT(ECPMaster *_sm,Reader *reader) : CPDataReaderListener(_sm)
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
            ControlLogError("Failed to read ECP data\n");
            return;
        } else if (retcode != DDS_RETCODE_OK) {
            // TODO: Error logging
            ControlLogError("ECP read failed with return code 5d\n",retcode);
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
                ev = new EventKind(&data_seq[i],&info_seq[i]);
                sm->PostEvent(ev);
            }
        }
        
        retcode = m_reader->return_loan(data_seq, info_seq);
        if (retcode != DDS_RETCODE_OK) {
            // TODO: Error logging
            ControlLogError("ECP return_loan failed with return code %d\n",retcode);
        }
    };
    
private:
    Reader *m_reader;
    ECPMaster *sm;    
};

typedef ECPMasterReaderListenerT<com::xvd::neuron::ecp::EventSeq,
com::xvd::neuron::ecp::EventDataReader,
ECPEventSessionEvent> ECPMasterEventReaderListener;

typedef ECPMasterReaderListenerT<com::xvd::neuron::ecp::MetricsSeq,
com::xvd::neuron::ecp::MetricsDataReader,
ECPEventSessionMetricsUpdate> ECPMasterMetricsReaderListener;

#endif
