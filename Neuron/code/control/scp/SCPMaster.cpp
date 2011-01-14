//!
//! \file SCPMaster.cpp
//!
//! \brief Implementation of SCPMaster
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#include "ndds_cpp.h"
#include "SCPEvent.h"
#include "SCPMasterObject.h"
#include "SCPMaster.h"

//! \class SCPMasterStateReaderListener
//!
//! \brief DDS on_data_available callback for the State data-reader
//!        This callback is handled differently because we want to track
//!        disposal of a state
//!
class SCPMasterStateReaderListener : public CPDataReaderListener
{
public:
    //! \brief Contstructor
    //!
    //! \param[in] sm - Master Object
    //!
    SCPMasterStateReaderListener(SCPMaster *_sm) : CPDataReaderListener(_sm)
    {
        sm = _sm;
    }

    //! \brief on_data_available
    //!
    //! \param[in] reader - Reader with available data
    //!
    void on_data_available(DDSDataReader* reader)
    {
        com::xvd::neuron::scp::StateSeq data_seq;
        DDS_SampleInfoSeq info_seq;
        DDS_ReturnCode_t retcode;
        int i;
        Event *ev;
        com::xvd::neuron::scp::StateDataReader *m_reader = com::xvd::neuron::scp::StateDataReader::narrow(reader);

        // NOTE: We do not track instance state for state/control/event
        retcode = m_reader->read(data_seq,
                                 info_seq,
                                 DDS_LENGTH_UNLIMITED,
                                 DDS_NOT_READ_SAMPLE_STATE,
                                 DDS_ANY_VIEW_STATE,
                                 DDS_ANY_INSTANCE_STATE);

        if (retcode == DDS_RETCODE_NO_DATA)
        {
            // TODO: Error logging
            ControlLogError("Failed to read SCP data\n");
            return;
        }
        else if (retcode != DDS_RETCODE_OK)
        {
            // TODO: Error logging
            ControlLogError("SCP read failed with return code %d\n",retcode);
            return;
        }

        for (i = 0; i < data_seq.length(); ++i)
        {
            if (info_seq[i].valid_data)
            {
                ev = new SCPEventSessionStateUpdate(&data_seq[i],&info_seq[i]);
                sm->PostEvent(ev);
            }
        }

        retcode = m_reader->return_loan(data_seq, info_seq);
        if (retcode != DDS_RETCODE_OK) {
            // TODO: Error logging
            ControlLogError("SCP return_loan failed with return code %d\n",retcode);
        }
    };

private:
    SCPMaster *sm;
};

//! \class SCPMaster
//!
//! \brief The SCPMaster attached to the DDS bus as a SCP master, enabling it to
//!        create/delete/update sessions, as well as listen to events, states and
//!        metrics associated with a session
//!
//! \param [in] Eventhandler - Who is going to handle events
//! \param [in] _srcId       - SCPMaster id
//! \param [in] domainId     - Domain to attach to
//! \param [in] name         - Name of the SCP Master object
//! \param [in] qosProfile   - Qos profile to use
//!
SCPMaster::SCPMaster(EventHandler *q,
                     int _srcId,
                     int domainId,
                     const char *name,
                     map<string,string> &PropertyPairs,
                     map<string,DDS_Boolean> &PropagateDiscoveryFlags,
                     const char *qosProfile) :
CPMasterT<
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
com::xvd::neuron::scp::MetricsTypeSupport>(q,_srcId,domainId,name,_srcId,PropertyPairs,PropagateDiscoveryFlags,qosProfile)
{
    DDS_ReturnCode_t retcode;

    SCPMasterMetricsReaderListener *metricsListener = new SCPMasterMetricsReaderListener(this,metricsReader);

    if (metricsListener == NULL)
    {
        //TODO: Replace with real error logging
        ControlLogError("Failed to create SCP metrics listener\n");
        throw DDS_RETCODE_BAD_PARAMETER;
    }

    retcode = metricsReader->set_listener(metricsListener, DDS_STATUS_MASK_ALL);
    if (retcode != DDS_RETCODE_OK)
    {
        //TODO: Replace with real error logging
        ControlLogError("Failed to set metrics listener\n");
        throw DDS_RETCODE_BAD_PARAMETER;
    }

    SCPMasterEventReaderListener *eventListener = new SCPMasterEventReaderListener(this,eventReader);
    if (eventListener == NULL)
    {
        //TODO: Replace with real error logging
        ControlLogError("Failed to create SCP event listener\n");
        throw DDS_RETCODE_BAD_PARAMETER;
    }

    retcode = eventReader->set_listener(eventListener, DDS_STATUS_MASK_ALL);
    if (retcode != DDS_RETCODE_OK)
    {
        //TODO: Replace with real error logging
        ControlLogError("Failed to set metrics listener\n");
        throw DDS_RETCODE_BAD_PARAMETER;
    }

    SCPMasterStateReaderListener *stateListener = new SCPMasterStateReaderListener(this);
    if (stateListener == NULL)
    {
        //TODO: Replace with real error logging
        ControlLogError("Failed to create SCP state listener\n");
        throw DDS_RETCODE_BAD_PARAMETER;
    }

    retcode = stateReader->set_listener(stateListener, DDS_STATUS_MASK_ALL);
    if (retcode != DDS_RETCODE_OK)
    {
        //TODO: Replace with real error logging
        ControlLogError("Failed to set SCP state listener\n");
        throw DDS_RETCODE_BAD_PARAMETER;
    }

    // Enable all entities
    controlWriter->enable();
    metricsReader->enable();
    stateReader->enable();
    eventReader->enable();
}

//! \class SCPMaster
//!
//! \brief Destructor
//!
SCPMaster::~SCPMaster()
{
    SCPMasterEventReaderListener *eventListener = NULL;
    SCPMasterMetricsReaderListener *metricsListener = NULL;
    SCPMasterStateReaderListener *stateListener = NULL;

    if (eventReader)
    {
        eventListener = (SCPMasterEventReaderListener*)eventReader->get_listener();
        eventReader->set_listener(NULL,DDS_STATUS_MASK_NONE);
        if (eventListener != NULL)
        {
            delete eventListener;
        }
    }

    if (stateReader)
    {
        stateListener = (SCPMasterStateReaderListener*)stateReader->get_listener();
        stateReader->set_listener(NULL,DDS_STATUS_MASK_NONE);
        if (stateListener != NULL)
        {
            delete stateListener;
        }
    }

    if (metricsReader)
    {
        metricsListener = (SCPMasterMetricsReaderListener*)metricsReader->get_listener();
        metricsReader->set_listener(NULL,DDS_STATUS_MASK_NONE);
        if (metricsListener != NULL)
        {
            delete metricsListener;
        }
    }
}

//!
//! \brief Create a SCP master object, managing _one_ session on the SCP. Note that
//!        no check is made if the session id already exists
//!
//! \param [in] sid - Session ID
//!
//! \return new session object on success, NULL on failure
//!
SCPMasterObject* SCPMaster::CreateMasterObject(int sid)
{
    SCPMasterObject *s = NULL;
    DDS_InstanceHandle_t h1 = DDS_HANDLE_NIL,
                         h2 = DDS_HANDLE_NIL,
                         h3 = DDS_HANDLE_NIL,
                         h4 = DDS_HANDLE_NIL;

    control->srcId = srcId;
    control->sessionId = sid;
    h1 = controlWriter->register_instance(*control);
    if (DDS_InstanceHandle_is_nil(&h1))
    {
        //TODO: Replace with real error logging
        ControlLogError("Failed create SCP control instance handle\n");
        goto done;
    }

    s = new SCPMasterObject(this,srcId,sid,h1,h2,h3,h4);

done:

    return s;
}

//!
//! \brief Delete a SCP Master Object create with CreateMasterObject
//!CreateMasterObject
//! \param [in] session - Session object allocated with CreateMasterObject
//!
//! \return true on success, false on faliure
//!
bool SCPMaster::DeleteMasterObject(SCPMasterObject* aSession)
{

    DDS_ReturnCode_t retcode;
    bool retval = false;

    retcode = controlWriter->get_key_value(*control,aSession->GetControlInstanceHandle());
    if (retcode != DDS_RETCODE_OK)
    {
        //TODO: Replace with real error logging
        ControlLogError("Failed get SCP control instance handle\n");
        goto done;
    }

    retcode = controlWriter->dispose(*control,aSession->GetControlInstanceHandle());
    if (retcode != DDS_RETCODE_OK)
    {
        //TODO: Replace with real error logging
        ControlLogError("Failed dispose SCP control instance handle\n");
        goto done;
    }

    delete aSession;
    retval = true;

done:

    return retval;
}

//! Return the State instance handle for a slave
//! \param[in] dstId  Slave ID of session
//! \param[in] sid    SessionID of session
//!
DDS_InstanceHandle_t SCPMaster::GetMasterObjectStateHandle(int dstId,int sid)
{
    state->srcId = dstId;
    state->sessionId = sid;

    return stateReader->lookup_instance(*state);
}

//! Return the Event instance handle for a slave
//! \param[in] dstId  Slave ID of session
//! \param[in] sid    SessionID of session
//!
DDS_InstanceHandle_t SCPMaster::GetMasterObjectEventHandle(int dstId, int sid)
{
    event->srcId = dstId;
    event->sessionId = sid;

    return eventReader->lookup_instance(*event);
}

//! Return the Metrics instance handle for a slave
//! \param[in] dstId  Slave ID of session
//! \param[in] sid    SessionID of session
//!
DDS_InstanceHandle_t SCPMaster::GetMasterObjectMetricsHandle(int dstId,int sid)
{
    metrics->srcId = dstId;
    metrics->sessionId = sid;

    return metricsReader->lookup_instance(*metrics);
}
