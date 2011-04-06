//!
//! \file ACPMaster.h
//!
//! \brief Defintion of Session Control Plane (ACP) Master
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#ifndef ACP_MASTER_H_
#define ACP_MASTER_H_

#include "ACPEvent.h"
#include "CPInterfaceT.h"
#include "ACPMasterObject.h"

//!
//! \class ACPMaster
//!
//! \brief ACP Master. This class attaches to the ACP plane as a master.
//!
//! Details: A ACP Master implements the following functions:
//!          - It is allowed to manage a session through the Control interface
//!          - It tracks the state of a session through the State, Event and
//!            Metrics interface.
//!          An application that wishes to create a new session must create it
//!          using the CreateMasterObject. This object represents a session in
//!          the ACP. Conversely, a session must be deleted from the ACP using 
//!          the DeleteMasterObject.
//!
//! \todo GetMasterObjectNNN is not to be called directly.
//!
class ACPMaster : public CPMasterT<
ACPMasterObject,
com::xvd::neuron::acp::ControlDataWriter,
com::xvd::neuron::acp::StateDataReader,
com::xvd::neuron::acp::EventDataReader,
com::xvd::neuron::acp::MetricsDataReader,
com::xvd::neuron::acp::EventSeq,
com::xvd::neuron::acp::StateSeq,
com::xvd::neuron::acp::MetricsSeq,
com::xvd::neuron::acp::Metrics,
com::xvd::neuron::acp::Event,
com::xvd::neuron::acp::State,
com::xvd::neuron::acp::Control,
com::xvd::neuron::acp::ControlTypeSupport,
com::xvd::neuron::acp::EventTypeSupport,
com::xvd::neuron::acp::StateTypeSupport,
com::xvd::neuron::acp::MetricsTypeSupport> 
{
public:
    
    //! Constructor for the ACPMaster object
    //!
    //! \param[in] eh          Event-handler for all events received
    //! \param[in] srcId       Unique ID for the ACPMaster in the ACP
    //! \param[in] qosProfile  RTI DDS QoS profile to use
    ACPMaster(EventHandler *eh,int srcId, int domainId, const char*name,map<string,string> &PropertyPairs,
              map<string,DDS_Boolean> &PropagateDiscoveryFlags,const char *qosProfile);
    
    //! Desstructor for the ACPMaster object
    //!
    ~ACPMaster();
    
    virtual ACPMasterObject* CreateMasterObject(int sid);
    
    virtual bool DeleteMasterObject(ACPMasterObject* aSession);
};

template<class DataSeq, class Reader,class EventKind>
class ACPMasterReaderListenerT : public CPDataReaderListener
{
public:
    ACPMasterReaderListenerT(ACPMaster *_sm,Reader *reader) : CPDataReaderListener(_sm)
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
            ControlLogError("Failed to read ACP data\n");
            return;
        } else if (retcode != DDS_RETCODE_OK) {
            ControlLogError("ACP read failed with return code 5d\n",retcode);
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
            ControlLogError("ACP return_loan failed with return code %d\n",retcode);
        }
    };
    
private:
    Reader *m_reader;
    ACPMaster *sm;    
};

typedef ACPMasterReaderListenerT<com::xvd::neuron::acp::EventSeq,
com::xvd::neuron::acp::EventDataReader,
ACPEventSessionEvent> ACPMasterEventReaderListener;

typedef ACPMasterReaderListenerT<com::xvd::neuron::acp::MetricsSeq,
com::xvd::neuron::acp::MetricsDataReader,
ACPEventSessionMetricsUpdate> ACPMasterMetricsReaderListener;

#endif
