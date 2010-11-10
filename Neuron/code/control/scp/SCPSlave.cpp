#include "ndds_cpp.h"
#include "neuroncommon.h"
#include "SCPEvent.h"
#include "SCPSlaveObject.h"
#include "SCPSlave.h"

SCPSlaveControlReaderListener::SCPSlaveControlReaderListener(SCPSlave *_sl,
            com::xvd::neuron::session::ControlDataReader *reader) :CPDataReaderListener(_sl)    
{
    m_reader = reader;
    sl = _sl;
}

void SCPSlaveControlReaderListener::on_data_available(DDSDataReader* reader)
{
    com::xvd::neuron::session::ControlSeq data_seq;
    DDS_SampleInfoSeq info_seq;
    DDS_ReturnCode_t retcode;
    int i;
    SCPSlaveObject *slave;
    Event *ev;
    com::xvd::neuron::session::Control *control;
    
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
                ev = new SCPEventNewSession(slave);
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
                    control = com::xvd::neuron::session::ControlTypeSupport::create_data();

                    m_reader->get_key_value(*control,info_seq[i].instance_handle);
                    
                    ev = new SCPEventDeleteSession(control->sessionId);
                    
                    com::xvd::neuron::session::ControlTypeSupport::delete_data(control);

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
CPSlaveT<com::xvd::neuron::session::ControlTypeSupport,
com::xvd::neuron::session::EventTypeSupport,
com::xvd::neuron::session::StateTypeSupport,
com::xvd::neuron::session::MetricsTypeSupport>(domainId,_srcId,qosProfile)
{
    controlReader = com::xvd::neuron::session::ControlDataReader::narrow(m_controlReader);
    if (controlReader == NULL)
    {
        return;
    }
    
    m_controlReader->set_listener(new SCPSlaveControlReaderListener(this, controlReader),DDS_STATUS_MASK_ALL);
    
    stateWriter = com::xvd::neuron::session::StateDataWriter::narrow(m_stateWriter);
    if (stateWriter == NULL) 
    {
        return;
    }
    
    eventWriter = com::xvd::neuron::session::EventDataWriter::narrow(m_eventWriter);
    if (eventWriter == NULL) 
    {
        return;
    }
    
    metricsWriter = com::xvd::neuron::session::MetricsDataWriter::narrow(m_metricsWriter);
    if (metricsWriter == NULL) 
    {
        return;
    }
    
    state = com::xvd::neuron::session::StateTypeSupport::create_data();
    if (state == NULL)
    {
        //TODO: Error log
        return;
    }
    
    event = com::xvd::neuron::session::EventTypeSupport::create_data();
    if (event == NULL) 
    {
        //TODO: Error log
        return;
    }
    
    metrics = com::xvd::neuron::session::MetricsTypeSupport::create_data();
    if (metrics == NULL)
    {
        //TODO: Error log
        return;
    }
    
    srcId = _srcId;
    upper = q;
    
    // The Slave should enter a state of INIT when it is done
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
    
    if (state != NULL)
    {
        com::xvd::neuron::session::StateTypeSupport::delete_data(state);
    }
    
    if (event != NULL)
    {
        com::xvd::neuron::session::EventTypeSupport::delete_data(event);
    }
    
    if (metrics != NULL)
    {
        com::xvd::neuron::session::MetricsTypeSupport::delete_data(metrics);
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

bool SCPSlave::Send(com::xvd::neuron::session::State *state, DDS_InstanceHandle_t ih)
{
    DDS_ReturnCode_t retcode;
    retcode = stateWriter->write(*state, ih);
    if (retcode != DDS_RETCODE_OK)
    {
        return false;
    }
    return true;
}

bool SCPSlave::Send(com::xvd::neuron::session::Event *event,DDS_InstanceHandle_t ih)
{
    DDS_ReturnCode_t retcode;
    retcode = eventWriter->write(*event, ih);
    if (retcode != DDS_RETCODE_OK)
    {
        return false;
    }
    return true;
}

bool SCPSlave::Send(com::xvd::neuron::session::Metrics *metrics, DDS_InstanceHandle_t ih)
{
    DDS_ReturnCode_t retcode;
    retcode = metricsWriter->write(*metrics, ih);
    if (retcode != DDS_RETCODE_OK)
    {
        return false;
    }
    return true;
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

bool SCPSlave::PostEvent(Event *ev)
{
    upper->SignalEvent(ev);
    return true;
}
