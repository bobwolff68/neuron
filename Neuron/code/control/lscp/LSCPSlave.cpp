#include "ndds_cpp.h"
#include "neuroncommon.h"
#include "LSCPEvent.h"
#include "LSCPSlaveObject.h"
#include "LSCPSlave.h"

LSCPSlaveControlReaderListener::LSCPSlaveControlReaderListener(LSCPSlave *_sl,
            com::xvd::neuron::lscp::ControlDataReader *reader) :CPDataReaderListener(_sl)    
{
    m_reader = reader;
    sl = _sl;
}

void LSCPSlaveControlReaderListener::on_data_available(DDSDataReader* reader)
{
    com::xvd::neuron::lscp::ControlSeq data_seq;
    DDS_SampleInfoSeq info_seq;
    DDS_ReturnCode_t retcode;
    int i;
    LSCPSlaveObject *slave;
    Event *ev;
    com::xvd::neuron::lscp::Control *control;
    
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
                ev = new LSCPEventNewSession(slave,&data_seq[i]);
                sl->PostEvent(ev);
                break;
            case DDS_NOT_NEW_VIEW_STATE:
                if (!info_seq[i].valid_data) {
                    // TODO: Error logging
                } else {
                    ev = new LSCPEventUpdateSession(&data_seq[i]);
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
                    control = com::xvd::neuron::lscp::ControlTypeSupport::create_data();

                    m_reader->get_key_value(*control,info_seq[i].instance_handle);
                    
                    ev = new LSCPEventDeleteSession(control->sessionId);
                    
                    com::xvd::neuron::lscp::ControlTypeSupport::delete_data(control);

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

LSCPSlave::LSCPSlave(EventHandler *q,int _srcId, int domainId, const char *qosProfile) : 
CPSlaveT<
LSCPSlaveObject,
com::xvd::neuron::lscp::ControlDataReader,
com::xvd::neuron::lscp::StateDataWriter,
com::xvd::neuron::lscp::EventDataWriter,
com::xvd::neuron::lscp::MetricsDataWriter,
com::xvd::neuron::lscp::State,
com::xvd::neuron::lscp::Event,
com::xvd::neuron::lscp::Metrics,
com::xvd::neuron::lscp::ControlTypeSupport,
com::xvd::neuron::lscp::EventTypeSupport,
com::xvd::neuron::lscp::StateTypeSupport,
com::xvd::neuron::lscp::MetricsTypeSupport>(q,_srcId,domainId,_srcId,qosProfile)
{
    m_controlReader->set_listener(new LSCPSlaveControlReaderListener(this, controlReader),DDS_STATUS_MASK_ALL);
}

LSCPSlave::~LSCPSlave()
{
    LSCPSlaveControlReaderListener *controlListener = NULL;
    
    if (m_controlReader)
    {
        controlListener = (LSCPSlaveControlReaderListener*)m_controlReader->get_listener();
        m_controlReader->set_listener(NULL,DDS_STATUS_MASK_NONE);
        if (controlListener != NULL)
        {
            delete controlListener;
        }
    }
}

// TODO: Allocate the session object
LSCPSlaveObject* LSCPSlave::CreateSlaveObject(int sid)
{
    LSCPSlaveObject *s = NULL;
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
    
    s = new LSCPSlaveObject(this,srcId,sid,h1,h2,h3);
    
done:
    return s;
}

bool LSCPSlave::DeleteSlaveObject(LSCPSlaveObject* aSession) 
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
