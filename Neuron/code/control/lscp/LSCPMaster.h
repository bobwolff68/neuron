//!
//! \file LSCPMaster.h
//!
//! \brief Defintion of Session Control Plane (LSCP) Master
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#ifndef LSCP_MASTER_H_
#define LSCP_MASTER_H_

#include "LSCPEvent.h"
#include "CPInterfaceT.h"
#include "LSCPMasterObject.h"

//!
//! \class LSCPMaster
//!
//! \brief LSCP Master. This class attaches to the LSCP plane as a master.
//!
//! Details: A LSCP Master implements the following functions:
//!          - It is allowed to manage a session through the Control interface
//!          - It tracks the state of a session through the State, Event and
//!            Metrics interface.
//!          An application that wishes to create a new session must create it
//!          using the CreateMasterObject. This object represents a session in
//!          the LSCP. Conversely, a session must be deleted from the LSCP using 
//!          the DeleteMasterObject.
//!
//! \todo GetMasterObjectNNN is not to be called directly.
//!
class LSCPMaster : public CPMasterT<
LSCPMasterObject,
com::xvd::neuron::lscp::ControlDataWriter,
com::xvd::neuron::lscp::StateDataReader,
com::xvd::neuron::lscp::EventDataReader,
com::xvd::neuron::lscp::MetricsDataReader,
com::xvd::neuron::lscp::EventSeq,
com::xvd::neuron::lscp::StateSeq,
com::xvd::neuron::lscp::MetricsSeq,
com::xvd::neuron::lscp::Metrics,
com::xvd::neuron::lscp::Event,
com::xvd::neuron::lscp::State,
com::xvd::neuron::lscp::Control,
com::xvd::neuron::lscp::ControlTypeSupport,
com::xvd::neuron::lscp::EventTypeSupport,
com::xvd::neuron::lscp::StateTypeSupport,
com::xvd::neuron::lscp::MetricsTypeSupport> 
{
public:
    
    //! Constructor for the LSCPMaster object
    //!
    //! \param[in] eh          Event-handler for all events received
    //! \param[in] srcId       Unique ID for the LSCPMaster in the LSCP
    //! \param[in] qosProfile  RTI DDS QoS profile to use
    LSCPMaster(EventHandler *eh,int srcId, int domainId, const char*, map<string,string> &PropertyPairs,
               map<string,DDS_Boolean> &PropagateDiscoveryFlags, const char *qosProfile);
    
    //! Desstructor for the LSCPMaster object
    //!
    ~LSCPMaster();
    
    virtual LSCPMasterObject* CreateMasterObject(int sid);
    
    virtual bool DeleteMasterObject(LSCPMasterObject* aSession);
};

template<class DataSeq, class Reader,class EventKind>
class LSCPMasterReaderListenerT : public CPDataReaderListener
{
public:
    LSCPMasterReaderListenerT(LSCPMaster *_sm,Reader *reader) : CPDataReaderListener(_sm)
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
            ControlLogError("Failed to read LSCP data\n");
            return;
        } else if (retcode != DDS_RETCODE_OK) {
            // TODO: Error logging
            ControlLogError("LSCP read failed with return code 5d\n",retcode);
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
        if (retcode != DDS_RETCODE_OK) 
        {
            // TODO: Error logging
            ControlLogError("ECP return_loan failed with return code %d\n",retcode);
        }
    };
    
private:
    Reader *m_reader;
    LSCPMaster *sm;    
};

typedef LSCPMasterReaderListenerT<com::xvd::neuron::lscp::EventSeq,
com::xvd::neuron::lscp::EventDataReader,
LSCPEventSessionEvent> LSCPMasterEventReaderListener;

typedef LSCPMasterReaderListenerT<com::xvd::neuron::lscp::MetricsSeq,
com::xvd::neuron::lscp::MetricsDataReader,
LSCPEventSessionMetricsUpdate> LSCPMasterMetricsReaderListener;

#endif
