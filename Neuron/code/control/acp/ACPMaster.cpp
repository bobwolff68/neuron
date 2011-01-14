//!
//! \file ACPMaster.cpp
//!
//! \brief Implementation of LSCPMaster
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#include "ndds_cpp.h"
#include "ACPEvent.h"
#include "ACPMasterObject.h"
#include "ACPMaster.h"

//! \class ACPMasterStateReaderListener
//!
//! \brief DDS on_data_available callback for the State data-reader
//!        This callback is handled differently because we want to track
//!        disposal of a state
//!
class ACPMasterStateReaderListener : public CPDataReaderListener
{
public:
    //! \brief Contstructor
    //!
    //! \param[in] sm - Master Object
    //!
    ACPMasterStateReaderListener(ACPMaster *_sm) : CPDataReaderListener(_sm)
    {
        sm = _sm;
    }

    //! \brief on_data_available
    //!
    //! \param[in] reader - Reader with available data
    //!
    void on_data_available(DDSDataReader* reader)
    {
        com::xvd::neuron::acp::StateSeq data_seq;
        DDS_SampleInfoSeq info_seq;
        DDS_ReturnCode_t retcode;
        int i;
        Event *ev;
        com::xvd::neuron::acp::StateDataReader *m_reader = com::xvd::neuron::acp::StateDataReader::narrow(reader);
        com::xvd::neuron::acp::State *state;

        // NOTE: We do not track instance state for state/control/event
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
            ControlLogError("LSCP read failed with return code %d\n",retcode);
            return;
        }

        for (i = 0; i < data_seq.length(); ++i) {

            switch (info_seq[i].instance_state)
            {
                case DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE:
                    state = com::xvd::neuron::acp::StateTypeSupport::create_data();
                    m_reader->get_key_value(*state, info_seq[i].instance_handle);
                    ev = new ACPEventSessionStateLost(state->srcId);
                    sm->PostEvent(ev);
                    com::xvd::neuron::acp::StateTypeSupport::delete_data(state);
                    break;
                default:
                    break;
            }

            if (info_seq[i].valid_data)
            {
                ev = new ACPEventSessionStateUpdate(&data_seq[i],&info_seq[i]);
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
    ACPMaster *sm;
};

ACPMaster::ACPMaster(EventHandler *q,int _srcId, int domainId, const char *name, map<string,string> &PropertyPairs,
                     map<string,DDS_Boolean> &PropagateDiscoveryFlags, const char *qosProfile) :
CPMasterT<
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
com::xvd::neuron::acp::MetricsTypeSupport>(q,_srcId,domainId,name,_srcId,PropertyPairs,PropagateDiscoveryFlags,qosProfile)
{
    DDS_ReturnCode_t retcode;

    ACPMasterMetricsReaderListener *metricsListener = new ACPMasterMetricsReaderListener(this,metricsReader);
    if (metricsListener == NULL)
    {
        //TODO: Replace with real error logging
        ControlLogError("Failed to create ACP metrics listener\n");
        throw DDS_RETCODE_BAD_PARAMETER;
    }

    retcode = metricsReader->set_listener(metricsListener, DDS_STATUS_MASK_ALL);
    if (retcode != DDS_RETCODE_OK)
    {
        //TODO: Replace with real error logging
        ControlLogError("Failed to set ACP metrics listener\n");
        throw DDS_RETCODE_BAD_PARAMETER;
    }
    ACPMasterEventReaderListener *eventListener = new ACPMasterEventReaderListener(this,eventReader);
    if (eventListener == NULL)
    {
        //TODO: Replace with real error logging
        ControlLogError("Failed to create ACP event listener\n");
        throw DDS_RETCODE_BAD_PARAMETER;
    }

    retcode = eventReader->set_listener(eventListener, DDS_STATUS_MASK_ALL);
    if (retcode != DDS_RETCODE_OK)
    {
        //TODO: Replace with real error logging
        ControlLogError("Failed to set ACP metrics listener\n");
        throw DDS_RETCODE_BAD_PARAMETER;
    }
    ACPMasterStateReaderListener *stateListener = new ACPMasterStateReaderListener(this);
    if (stateListener == NULL)
    {
        //TODO: Replace with real error logging
        ControlLogError("Failed to create ACP state listener\n");
        throw DDS_RETCODE_BAD_PARAMETER;
    }

    retcode = stateReader->set_listener(stateListener, DDS_STATUS_MASK_ALL);
    if (retcode != DDS_RETCODE_OK)
    {
        //TODO: Replace with real error logging
        ControlLogError("Failed to set LSCP state listener\n");
        throw DDS_RETCODE_BAD_PARAMETER;
    }

    // Enable all entities
    controlWriter->enable();
    metricsReader->enable();
    stateReader->enable();
    eventReader->enable();
}

ACPMaster::~ACPMaster()
{
    ACPMasterEventReaderListener *eventListener = NULL;
    ACPMasterMetricsReaderListener *metricsListener = NULL;
    ACPMasterStateReaderListener *stateListener = NULL;

    if (eventReader)
    {
        eventListener = (ACPMasterEventReaderListener*)eventReader->get_listener();
        eventReader->set_listener(NULL,DDS_STATUS_MASK_NONE);
        if (eventListener != NULL)
        {
            delete eventListener;
        }
    }

    if (stateReader)
    {
        stateListener = (ACPMasterStateReaderListener*)stateReader->get_listener();
        stateReader->set_listener(NULL,DDS_STATUS_MASK_NONE);
        if (stateListener != NULL)
        {
            delete stateListener;
        }
    }

    if (metricsReader)
    {
        metricsListener = (ACPMasterMetricsReaderListener*)metricsReader->get_listener();
        metricsReader->set_listener(NULL,DDS_STATUS_MASK_NONE);
        if (metricsListener != NULL)
        {
            delete metricsListener;
        }
    }
}

ACPMasterObject* ACPMaster::CreateMasterObject(int sid)
{
    ACPMasterObject *s = NULL;
    DDS_InstanceHandle_t h1 = DDS_HANDLE_NIL,
                         h2 = DDS_HANDLE_NIL,
                         h3 = DDS_HANDLE_NIL,
                         h4 = DDS_HANDLE_NIL;

    control->srcId = srcId;
    control->dstId = sid;
    h1 = controlWriter->register_instance(*control);
    if (DDS_InstanceHandle_is_nil(&h1))
    {
        //TODO: Replace with real error logging
        ControlLogError("Failed create ACP control instance handle\n");
        goto done;
    }

    s = new ACPMasterObject(this,srcId,sid,h1,h2,h3,h4);

done:

    return s;
}

bool ACPMaster::DeleteMasterObject(ACPMasterObject* aSession)
{

    DDS_ReturnCode_t retcode;
    bool retval = false;

    retcode = controlWriter->get_key_value(*control,aSession->GetControlInstanceHandle());
    if (retcode != DDS_RETCODE_OK)
    {
        //TODO: Replace with real error logging
        ControlLogError("Failed get ACP control instance handle\n");
        goto done;
    }

    retcode = controlWriter->dispose(*control,aSession->GetControlInstanceHandle());
    if (retcode != DDS_RETCODE_OK)
    {
        //TODO: Replace with real error logging
        ControlLogError("Failed dispose ACP control instance handle\n");
        goto done;
    }

    delete aSession;

    retval = true;

done:

    return retval;
}



