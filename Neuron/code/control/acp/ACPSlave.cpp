#include "ndds_cpp.h"
#include "neuroncommon.h"
#include "ACPEvent.h"
#include "ACPSlaveObject.h"
#include "ACPSlave.h"

ACPSlaveControlReaderListener::ACPSlaveControlReaderListener(ACPSlave *_sl,
            com::xvd::neuron::acp::ControlDataReader *reader) :CPDataReaderListener(_sl)    
{
    m_reader = reader;
    sl = _sl;
}

void ACPSlaveControlReaderListener::on_data_available(DDSDataReader* reader)
{
    com::xvd::neuron::acp::ControlSeq data_seq;
    DDS_SampleInfoSeq info_seq;
    DDS_ReturnCode_t retcode;
    int i;
    ACPSlaveObject *slave;
    Event *ev;
    com::xvd::neuron::acp::Control *control;
    
    printf("data rx\n");
    retcode = m_reader->take(data_seq, 
                             info_seq, 
                             DDS_LENGTH_UNLIMITED,
                             DDS_ANY_SAMPLE_STATE, 
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
                slave = sl->CreateSlaveObject(data_seq[i].srcId);
                ev = new ACPEventNewSession(slave,&data_seq[i]);
                sl->PostEvent(ev);
                break;
            case DDS_NOT_NEW_VIEW_STATE:
                if (!info_seq[i].valid_data) {
                    // TODO: Error logging
                } else {
                    ev = new ACPEventUpdateSession(&data_seq[i]);
                    sl->PostEvent(ev);                    
                }
                break;
            default:
                    // TODO: Error logging
                break;
        }

        switch (info_seq[i].instance_state) {
            case DDS_ALIVE_INSTANCE_STATE:
                if (info_seq[i].valid_data) {
                    // TODO: Error logging
                } else {
                }
                break;
            case DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE:
                if (info_seq[i].valid_data) {
                    // TODO 
                } else {
                }
                break;
            case DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE:
                if (info_seq[i].valid_data) {
                    // TODO: Error logging
                } else {
                    control = com::xvd::neuron::acp::ControlTypeSupport::create_data();

                    m_reader->get_key_value(*control,info_seq[i].instance_handle);
                    
                    ev = new ACPEventDeleteSession(control->srcId);
                    
                    com::xvd::neuron::acp::ControlTypeSupport::delete_data(control);

                    sl->PostEvent(ev);
                }
                break;
            default:
                // TODO: Error logging
                break;
        }
    }
    
    retcode = m_reader->return_loan(data_seq, info_seq);
    if (retcode != DDS_RETCODE_OK) {
        // TODO: Error logging
    }
};

ACPSlave::ACPSlave(EventHandler *q,int _srcId, int domainId, const char *qosProfile) : 
CPSlaveT<
ACPSlaveObject,
com::xvd::neuron::acp::ControlDataReader,
com::xvd::neuron::acp::StateDataWriter,
com::xvd::neuron::acp::EventDataWriter,
com::xvd::neuron::acp::MetricsDataWriter,
com::xvd::neuron::acp::State,
com::xvd::neuron::acp::Event,
com::xvd::neuron::acp::Metrics,
com::xvd::neuron::acp::ControlTypeSupport,
com::xvd::neuron::acp::EventTypeSupport,
com::xvd::neuron::acp::StateTypeSupport,
com::xvd::neuron::acp::MetricsTypeSupport>(q,_srcId,domainId,_srcId,qosProfile)
{
    m_controlReader->set_listener(new ACPSlaveControlReaderListener(this, controlReader),DDS_STATUS_MASK_ALL);
}

ACPSlave::~ACPSlave()
{
    ACPSlaveControlReaderListener *controlListener = NULL;
    
    if (m_controlReader)
    {
        controlListener = (ACPSlaveControlReaderListener*)m_controlReader->get_listener();
        m_controlReader->set_listener(NULL,DDS_STATUS_MASK_NONE);
        if (controlListener != NULL)
        {
            delete controlListener;
        }
    }
}

// TODO: Allocate the session object
ACPSlaveObject* ACPSlave::CreateSlaveObject(int sid)
{
    ACPSlaveObject *s = NULL;
    DDS_InstanceHandle_t h1 = DDS_HANDLE_NIL,
                         h2 = DDS_HANDLE_NIL,
                         h3 = DDS_HANDLE_NIL;    
        
    state->srcId = srcId;
    h1 = stateWriter->register_instance(*state);
    if (DDS_InstanceHandle_is_nil(&h1)) 
    {
        //TODO: Error log
        printf("h1 is nil\n");
        goto done;
    }
    
    event->srcId = srcId;
    h2 = eventWriter->register_instance(*event);
    if (DDS_InstanceHandle_is_nil(&h2)) 
    {
        //TODO: Error log
        printf("h2 is nil\n");
        goto done;
    }
    
    metrics->srcId = srcId;
    h3 = metricsWriter->register_instance(*metrics);
    if (DDS_InstanceHandle_is_nil(&h3)) 
    {
        //TODO: Error log
        printf("h3 is nil\n");
        goto done;
    }
    
    s = new ACPSlaveObject(this,srcId,sid,h1,h2,h3);
    
done:
    return s;
}

bool ACPSlave::DeleteSlaveObject(ACPSlaveObject* aSession) 
{
    DDS_ReturnCode_t retcode;
    bool retval = false;
    
    retcode = eventWriter->get_key_value(*event,aSession->GetEventInstanceHandle());
    if (retcode != DDS_RETCODE_OK) 
    {
        //TODO: Error log
        goto done;
    }
    
    retcode = eventWriter->dispose(*event,aSession->GetEventInstanceHandle());
    if (retcode != DDS_RETCODE_OK) 
    {
        //TODO: Error log
        goto done;
    }
    
    retcode = stateWriter->get_key_value(*state,aSession->GetStateInstanceHandle());
    if (retcode != DDS_RETCODE_OK) 
    {
        //TODO: Error log
        goto done;
    }
    
    retcode = stateWriter->dispose(*state,aSession->GetStateInstanceHandle());
    if (retcode != DDS_RETCODE_OK) 
    {
        //TODO: Error log
        goto done;
    }

    retcode = metricsWriter->get_key_value(*metrics,
                                           aSession->GetMetricsInstanceHandle());
    if (retcode != DDS_RETCODE_OK) 
    {
        //TODO: Error log
        goto done;
    }
    
    retcode = metricsWriter->dispose(*metrics,
                                     aSession->GetMetricsInstanceHandle());
    if (retcode != DDS_RETCODE_OK) 
    {
        //TODO: Error log
        goto done;
    }
    
    delete aSession;
    retval = true;
    
done:
        
    return retval;
}
