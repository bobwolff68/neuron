#include "ndds_cpp.h"
#include "neuroncommon.h"
#include "ECPEvent.h"
#include "ECPSlaveObject.h"
#include "ECPSlave.h"

ECPSlaveControlReaderListener::ECPSlaveControlReaderListener(ECPSlave *_sl,
            com::xvd::neuron::ecp::ControlDataReader *reader) :CPDataReaderListener(_sl)    
{
    m_reader = reader;
    sl = _sl;
}

void ECPSlaveControlReaderListener::on_data_available(DDSDataReader* reader)
{
    com::xvd::neuron::ecp::ControlSeq data_seq;
    DDS_SampleInfoSeq info_seq;
    DDS_ReturnCode_t retcode;
    int i;
    ECPSlaveObject *slave;
    Event *ev;
    com::xvd::neuron::ecp::Control *control;
    
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
                slave = sl->CreateSlaveObject(data_seq[i].sessionId);
                ev = new ECPEventNewSession(slave,&data_seq[i]);
                sl->PostEvent(ev);
                break;
            case DDS_NOT_NEW_VIEW_STATE:
                if (!info_seq[i].valid_data) {
                    // TODO: Error logging
                } else {
                    ev = new ECPEventUpdateSession(&data_seq[i]);
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
                    control = com::xvd::neuron::ecp::ControlTypeSupport::create_data();

                    m_reader->get_key_value(*control,info_seq[i].instance_handle);
                    
                    ev = new ECPEventDeleteSession(control->sessionId);
                    
                    com::xvd::neuron::ecp::ControlTypeSupport::delete_data(control);

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

ECPSlave::ECPSlave(EventHandler *q,int _srcId, int domainId, const char *qosProfile) : 
CPSlaveT<
ECPSlaveObject,
com::xvd::neuron::ecp::ControlDataReader,
com::xvd::neuron::ecp::StateDataWriter,
com::xvd::neuron::ecp::EventDataWriter,
com::xvd::neuron::ecp::MetricsDataWriter,
com::xvd::neuron::ecp::State,
com::xvd::neuron::ecp::Event,
com::xvd::neuron::ecp::Metrics,
com::xvd::neuron::ecp::ControlTypeSupport,
com::xvd::neuron::ecp::EventTypeSupport,
com::xvd::neuron::ecp::StateTypeSupport,
com::xvd::neuron::ecp::MetricsTypeSupport>(q,_srcId,domainId,_srcId,qosProfile)
{
    m_controlReader->set_listener(new ECPSlaveControlReaderListener(this, controlReader),DDS_STATUS_MASK_ALL);
}

ECPSlave::~ECPSlave()
{
    ECPSlaveControlReaderListener *controlListener = NULL;
    
    if (m_controlReader)
    {
        controlListener = (ECPSlaveControlReaderListener*)m_controlReader->get_listener();
        m_controlReader->set_listener(NULL,DDS_STATUS_MASK_NONE);
        if (controlListener != NULL)
        {
            delete controlListener;
        }
    }
}

// TODO: Allocate the session object
ECPSlaveObject* ECPSlave::CreateSlaveObject(int sid)
{
    ECPSlaveObject *s = NULL;
    DDS_InstanceHandle_t h1 = DDS_HANDLE_NIL,
                         h2 = DDS_HANDLE_NIL,
                         h3 = DDS_HANDLE_NIL;    
    
    
    state->sessionId = sid;
    state->srcId = srcId;
    h1 = stateWriter->register_instance(*state);
    if (DDS_InstanceHandle_is_nil(&h1)) 
    {
        //TODO: Error log
        printf("h1 is nil\n");
        goto done;
    }
    
    event->sessionId = sid;
    event->srcId = srcId;
    h2 = eventWriter->register_instance(*event);
    if (DDS_InstanceHandle_is_nil(&h2)) 
    {
        //TODO: Error log
        printf("h2 is nil\n");
        goto done;
    }
    
    metrics->sessionId = sid;
    metrics->srcId = srcId;
    h3 = metricsWriter->register_instance(*metrics);
    if (DDS_InstanceHandle_is_nil(&h3)) 
    {
        //TODO: Error log
        printf("h3 is nil\n");
        goto done;
    }
    
    s = new ECPSlaveObject(this,srcId,sid,h1,h2,h3);
    
done:
    return s;
}

bool ECPSlave::DeleteSlaveObject(ECPSlaveObject* aSession) 
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
