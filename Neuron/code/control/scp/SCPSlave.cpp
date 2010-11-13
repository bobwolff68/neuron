#include "ndds_cpp.h"
#include "neuroncommon.h"
#include "SCPEvent.h"
#include "SCPSlaveObject.h"
#include "SCPSlave.h"

SCPSlaveControlReaderListener::SCPSlaveControlReaderListener(SCPSlave *_sl,
            com::xvd::neuron::scp::ControlDataReader *reader) :CPDataReaderListener(_sl)    
{
    m_reader = reader;
    sl = _sl;
}

void SCPSlaveControlReaderListener::on_data_available(DDSDataReader* reader)
{
    com::xvd::neuron::scp::ControlSeq data_seq;
    DDS_SampleInfoSeq info_seq;
    DDS_ReturnCode_t retcode;
    int i;
    SCPSlaveObject *slave;
    Event *ev;
    com::xvd::neuron::scp::Control *control;
    
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
                ev = new SCPEventNewSession(slave,&data_seq[i]);
                sl->PostEvent(ev);
                break;
            case DDS_NOT_NEW_VIEW_STATE:
                if (!info_seq[i].valid_data) {
                    // TODO: Error logging
                } else {
                    ev = new SCPEventUpdateSession(&data_seq[i]);
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
                    control = com::xvd::neuron::scp::ControlTypeSupport::create_data();

                    m_reader->get_key_value(*control,info_seq[i].instance_handle);
                    
                    ev = new SCPEventDeleteSession(control->sessionId);
                    
                    com::xvd::neuron::scp::ControlTypeSupport::delete_data(control);

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

SCPSlave::SCPSlave(EventHandler *q,int _srcId, int domainId, const char *qosProfile) : 
CPSlaveT<
SCPSlaveObject,
com::xvd::neuron::scp::ControlDataReader,
com::xvd::neuron::scp::StateDataWriter,
com::xvd::neuron::scp::EventDataWriter,
com::xvd::neuron::scp::MetricsDataWriter,
com::xvd::neuron::scp::State,
com::xvd::neuron::scp::Event,
com::xvd::neuron::scp::Metrics,
com::xvd::neuron::scp::ControlTypeSupport,
com::xvd::neuron::scp::EventTypeSupport,
com::xvd::neuron::scp::StateTypeSupport,
com::xvd::neuron::scp::MetricsTypeSupport>(q,_srcId,domainId,_srcId,qosProfile)
{
    m_controlReader->set_listener(new SCPSlaveControlReaderListener(this, controlReader),DDS_STATUS_MASK_ALL);
}

SCPSlave::~SCPSlave()
{
    SCPSlaveControlReaderListener *controlListener = NULL;
    
    if (m_controlReader)
    {
        controlListener = (SCPSlaveControlReaderListener*)m_controlReader->get_listener();
        m_controlReader->set_listener(NULL,DDS_STATUS_MASK_NONE);
        if (controlListener != NULL)
        {
            delete controlListener;
        }
    }
}

// TODO: Allocate the session object
SCPSlaveObject* SCPSlave::CreateSlaveObject(int sid)
{
    SCPSlaveObject *s = NULL;
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
    
    s = new SCPSlaveObject(this,srcId,sid,h1,h2,h3);
    
done:
    return s;
}

bool SCPSlave::DeleteSlaveObject(SCPSlaveObject* aSession) 
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
