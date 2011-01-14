//!
//! \file ECPMaster.cpp
//!
//! \brief Defintion of ECP Master Object
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#include "ndds_cpp.h"
#include "ECPEvent.h"
#include "ECPMasterObject.h"
#include "ECPMaster.h"

//! \class ECPMasterStateReaderListener
//!
//! \brief DDS on_data_available callback for the State data-reader
//!        This callback is handled differently because we want to track
//!        disposal of a state
//!
class ECPMasterStateReaderListener : public CPDataReaderListener
{
public:
    //! \brief Contstructor
    //!
    //! \param[in] sm - Master Object
    //!    
    ECPMasterStateReaderListener(ECPMaster *_sm) : CPDataReaderListener(_sm)
    {
        sm = _sm;
    }
    
    //! \brief on_data_available
    //!
    //! \param[in] reader - Reader with available data
    //!        
    void on_data_available(DDSDataReader* reader)
    {
        com::xvd::neuron::ecp::StateSeq data_seq;
        DDS_SampleInfoSeq info_seq;
        DDS_ReturnCode_t retcode;
        int i;
        Event *ev;
        com::xvd::neuron::ecp::StateDataReader *m_reader = com::xvd::neuron::ecp::StateDataReader::narrow(reader);
        // NOTE: We do not track instance state for state/control/event
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
            ControlLogError("ECP read failed with return code %d\n",retcode);
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
                ev = new ECPEventSessionStateUpdate(&data_seq[i],&info_seq[i]);
                sm->PostEvent(ev);
            }
        }
        
        retcode = m_reader->return_loan(data_seq, info_seq);
        if (retcode != DDS_RETCODE_OK) {
            // TODO: Error logging
            ControlLogError("SCP return_loan failed with return code 5d\n",retcode);
        }
    };
    
private:
    ECPMaster *sm;    
};

ECPMaster::ECPMaster(EventHandler *q,int _srcId, int domainId, const char *name,const char *qosProfile) : 
CPMasterT<
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
com::xvd::neuron::ecp::MetricsTypeSupport>(q,_srcId,domainId,name,_srcId,qosProfile)
{
    DDS_ReturnCode_t retcode;
    
    ECPMasterMetricsReaderListener *metricsListener = new ECPMasterMetricsReaderListener(this,metricsReader);
    if (metricsListener == NULL)
    {
        //TODO: Replace with real error logging
        ControlLogError("Failed to create ECP metrics listener\n");
        throw DDS_RETCODE_BAD_PARAMETER;
    }
    
    retcode = metricsReader->set_listener(metricsListener, DDS_STATUS_MASK_ALL);
    if (retcode != DDS_RETCODE_OK)
    {
        //TODO: Replace with real error logging
        ControlLogError("Failed to set ECP metrics listener\n");
        throw DDS_RETCODE_BAD_PARAMETER;
    }
    ECPMasterEventReaderListener *eventListener = new ECPMasterEventReaderListener(this,eventReader);
    if (eventListener == NULL)
    {
        //TODO: Replace with real error logging
        ControlLogError("Failed to create ECP event listener\n");
        throw DDS_RETCODE_BAD_PARAMETER;
    }
    
    retcode = eventReader->set_listener(eventListener, DDS_STATUS_MASK_ALL);
    if (retcode != DDS_RETCODE_OK)
    {
        //TODO: Replace with real error logging
        ControlLogError("Failed to set ECP event listener\n");
        throw DDS_RETCODE_BAD_PARAMETER;
    }
    ECPMasterStateReaderListener *stateListener = new ECPMasterStateReaderListener(this);
    if (stateListener == NULL) 
    {
        //TODO: Replace with real error logging
        ControlLogError("Failed to create ECP state listener\n");
        throw DDS_RETCODE_BAD_PARAMETER;
    }
    
    retcode = stateReader->set_listener(stateListener, DDS_STATUS_MASK_ALL);
    if (retcode != DDS_RETCODE_OK)
    {
        //TODO: Replace with real error logging
        ControlLogError("Failed to set ECP state listener\n");
        throw DDS_RETCODE_BAD_PARAMETER;
    }
    
    // Enable all entities
    controlWriter->enable();
    metricsReader->enable();
    stateReader->enable();
    eventReader->enable();    
}

ECPMaster::~ECPMaster()
{
    ECPMasterEventReaderListener *eventListener = NULL;
    ECPMasterMetricsReaderListener *metricsListener = NULL;
    ECPMasterStateReaderListener *stateListener = NULL;

    if (eventReader)
    {
        eventListener = (ECPMasterEventReaderListener*)eventReader->get_listener();
        eventReader->set_listener(NULL,DDS_STATUS_MASK_NONE);
        if (eventListener != NULL)
        {
            delete eventListener;
        }
    }

    if (stateReader)
    {
        stateListener = (ECPMasterStateReaderListener*)stateReader->get_listener();
        stateReader->set_listener(NULL,DDS_STATUS_MASK_NONE);
        if (stateListener != NULL)
        {
            delete stateListener;
        }
    }

    if (metricsReader)
    {
        metricsListener = (ECPMasterMetricsReaderListener*)metricsReader->get_listener();
        metricsReader->set_listener(NULL,DDS_STATUS_MASK_NONE);
        if (metricsListener != NULL)
        {
            delete metricsListener;
        }
    }
}

ECPMasterObject* ECPMaster::CreateMasterObject(int sid)
{
    ECPMasterObject *s = NULL;
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
        ControlLogError("Failed create ECP control instance handle\n");
        goto done;
    }
    
    s = new ECPMasterObject(this,srcId,sid,h1,h2,h3,h4);
    
done:
        
    return s;
}

bool ECPMaster::DeleteMasterObject(ECPMasterObject* aSession) 
{
    
    DDS_ReturnCode_t retcode;
    bool retval = false;
    
    retcode = controlWriter->get_key_value(*control,aSession->GetControlInstanceHandle());
    if (retcode != DDS_RETCODE_OK) 
    {
        //TODO: Replace with real error logging
        ControlLogError("Failed get ECP control instance handle\n");
        goto done;
    }

    retcode = controlWriter->dispose(*control,aSession->GetControlInstanceHandle());
    if (retcode != DDS_RETCODE_OK) 
    {  
        //TODO: Replace with real error logging
        ControlLogError("Failed dispose ECP control instance handle\n");
        goto done;    
    }
    
    delete aSession;
    retval = true;
    
done:
    
    return retval;
}



