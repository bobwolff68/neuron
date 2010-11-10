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
//!          - It is allowed to manage a session through the Control interface
//!          - It tracks the state of a session through the State, Event and
//!            Metrics interface.
//!          An application that wishes to create a new session must create it
//!          using the CreateMasterObject. This object represents a session in
//!          the SCP. Conversely, a session must be deleted from the SCP using 
//!          the DeleteMasterObject.
//!
//! \todo GetMasterObjectNNN is not to be called directly.
//!
class SCPMaster : public CPMasterT<com::xvd::neuron::session::ControlTypeSupport,
                                   com::xvd::neuron::session::EventTypeSupport,
                                   com::xvd::neuron::session::StateTypeSupport,
                                   com::xvd::neuron::session::MetricsTypeSupport> 
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
    
    //! Create a new Session Object
    //!
    //! \param[in] sessionId   Session ID
    SCPMasterObject* CreateMasterObject(int sessionId);

    //! Delete a Session object created with CreateMasterObject
    //!
    //! \param[in] so Session object to delete
    bool DeleteMasterObject(SCPMasterObject* so);

    //! Send a control topic
    //!
    //! \param[in] control control data to send on the SCP
    //!
    //! \todo. This method should not be exposed to applications
    bool Send(com::xvd::neuron::session::Control*,DDS_InstanceHandle_t ih);
    
    //! Return the current state for a particular session
    //!
    //! \param[in] instance Session instance to retrieve state for
    //! \param[out] state Contains state on succcessful return
    //! \return true on success, false on failure
    //!
    //! \todo. This method should not be exposed to applications    
    bool GetMasterObjectState(DDS_InstanceHandle_t instance,com::xvd::neuron::session::State*);

    //! Return the current state for a particular session
    //!
    //! \param[in] instance Session instance to retrieve Events for
    //! \param[out] events Contains events on succcessful return
    //! \return true on success, false on failure
    //!
    //! \todo. This method should not be exposed to applications        
    bool GetMasterObjectEvents(DDS_InstanceHandle_t instance,com::xvd::neuron::session::EventSeq*);

    //! Return the current state for a particular session
    //!
    //! \param[in] instance Session instance to retrieve Metrics for
    //! \param[out] metrics Contains events on succcessful return
    //! \return true on success, false on failure
    //!
    //! \todo. This method should not be exposed to applications            
    bool GetMasterObjectMetrics(DDS_InstanceHandle_t instance,com::xvd::neuron::session::MetricsSeq*);

    //! Return the current state for a particular session
    //!
    //! \param[in] ev new event
    //!
    //! \todo. This method should not be exposed to applications                
    virtual bool PostEvent(Event *ev);

private:
    //! \var srcId
    //! \brief Unique ID for the SCPMaster in the SCP
    int srcId;

    //! \var upper
    //! \brief Upper-layer event handle
    EventHandler *upper;
    
    //! \var controlWriter
    //! \brief DDS writer for control data
    com::xvd::neuron::session::ControlDataWriter *controlWriter;

    //! \var stateReader
    //! \brief DDS reader for state data
    com::xvd::neuron::session::StateDataReader *stateReader;

    //! \var eventReader
    //! \brief DDS reader for event data
    com::xvd::neuron::session::EventDataReader *eventReader;
    
    //! \var metricsReader
    //! \brief DDS reader for metrics data
    com::xvd::neuron::session::MetricsDataReader *metricsReader;
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

typedef SCPMasterReaderListenerT<com::xvd::neuron::session::StateSeq,
com::xvd::neuron::session::StateDataReader,
SCPEventSessionStateUpdate> SCPMasterStateReaderListener;

typedef SCPMasterReaderListenerT<com::xvd::neuron::session::EventSeq,
com::xvd::neuron::session::EventDataReader,
SCPEventSessionEvent> SCPMasterEventReaderListener;

typedef SCPMasterReaderListenerT<com::xvd::neuron::session::MetricsSeq,
com::xvd::neuron::session::MetricsDataReader,
SCPEventSessionMetricsUpdate> SCPMasterMetricsReaderListener;

#endif
