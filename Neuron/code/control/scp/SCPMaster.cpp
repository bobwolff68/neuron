#include "ndds_cpp.h"
#include "SCPEvent.h"
#include "SCPMasterObject.h"
#include "SCPMaster.h"

SCPMaster::SCPMaster(EventHandler *q,int _srcId, int domainId, const char *qosProfile) : 
CPMasterT<com::xvd::neuron::session::ControlTypeSupport,
com::xvd::neuron::session::EventTypeSupport,
com::xvd::neuron::session::StateTypeSupport,
com::xvd::neuron::session::MetricsTypeSupport>(domainId,_srcId,qosProfile)
{
    DDS_ReturnCode_t retcode;
    
    controlWriter = com::xvd::neuron::session::ControlDataWriter::narrow(m_controlWriter);
    if (controlWriter == NULL)
    {
        // TODO: Error handling
        return;
    }
    
    stateReader = com::xvd::neuron::session::StateDataReader::narrow(m_stateReader);
    if (stateReader == NULL) 
    {
        // TODO: Error handling
        return;
    }
    
    SCPMasterStateReaderListener *stateListener = new SCPMasterStateReaderListener(this,stateReader);
    if (stateListener == NULL) 
    {
        // TODO Error handling
        return;
    }
    
    retcode = m_stateReader->set_listener(stateListener, DDS_STATUS_MASK_ALL);
    if (retcode != DDS_RETCODE_OK)
    {
        // TODO Error handling
        return;
    }
    
    eventReader = com::xvd::neuron::session::EventDataReader::narrow(m_eventReader);
    if (eventReader == NULL) 
    {
        // TODO: Error handling
        return;
    }
    
    SCPMasterEventReaderListener *eventListener = new SCPMasterEventReaderListener(this,eventReader);
    if (eventListener == NULL)
    {
        // TODO: Error logging
        return;
    }
    
    retcode = m_eventReader->set_listener(eventListener, DDS_STATUS_MASK_ALL);
    if (retcode != DDS_RETCODE_OK)
    {
        // TODO Error handling
        return;
    }
    
    metricsReader = com::xvd::neuron::session::MetricsDataReader::narrow(m_metricsReader);
    if (metricsReader == NULL) 
    {
        // TODO: Error handling
        return;
    }  
    
    SCPMasterMetricsReaderListener *metricsListener = new SCPMasterMetricsReaderListener(this,metricsReader);
    if (metricsListener == NULL)
    {
        // TODO: Error handling
        return;
    }
    
    retcode = m_metricsReader->set_listener(metricsListener, DDS_STATUS_MASK_ALL);
    if (retcode != DDS_RETCODE_OK)
    {
        // TODO Error handling
        return;
    }
    
    srcId = _srcId;
    upper = q;
}

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

SCPMasterObject* SCPMaster::CreateMasterObject(int sid)
{
    SCPMasterObject *s = NULL;
    DDS_InstanceHandle_t h1 = DDS_HANDLE_NIL,
                         h2 = DDS_HANDLE_NIL,
                         h3 = DDS_HANDLE_NIL,
                         h4 = DDS_HANDLE_NIL;
    com::xvd::neuron::session::Control *control = NULL;
    com::xvd::neuron::session::State *state = NULL;
    com::xvd::neuron::session::Event *event = NULL;
    com::xvd::neuron::session::Metrics *metrics = NULL;
    
    state = com::xvd::neuron::session::StateTypeSupport::create_data();
    if (state == NULL)
    {
        //TODO: Error handling
        goto done;
    }
    
    event = com::xvd::neuron::session::EventTypeSupport::create_data();
    if (event == NULL) 
    {
        //TODO: Error handling
        goto done;
    }
    
    metrics = com::xvd::neuron::session::MetricsTypeSupport::create_data();
    if (metrics == NULL)
    {
        //TODO: Error handling
        goto done;
    }
    
    control = com::xvd::neuron::session::ControlTypeSupport::create_data();
    if (control == NULL)
    {
        //TODO: Error handling
        goto done;
    }

    control->srcId = srcId;
    control->sessionId = sid;
    h1 = controlWriter->register_instance(*control);
    if (DDS_InstanceHandle_is_nil(&h1)) 
    {
        //TODO: Error handling
        goto done;
    }
#if 0
    state->srcId = srcId;
    state->sessionId = sid;
    h2 = stateReader->lookup_instance(*state);
    if (DDS_InstanceHandle_is_nil(&h2)) 
    {
        //TODO: Error handling
        goto done;
    }

    event->srcId = srcId;
    event->sessionId = sid;
    h3 = eventReader->lookup_instance(*event);
    if (DDS_InstanceHandle_is_nil(&h3)) 
    {
        //TODO: Error handling
        goto done;
    }

    metrics->srcId = srcId;
    metrics->sessionId = sid;
    h4 = metricsReader->lookup_instance(*metrics);
    if (DDS_InstanceHandle_is_nil(&h4)) 
    {
        //TODO: Error handling
        goto done;
    }
#endif
    
    s = new SCPMasterObject(this,srcId,sid,h1,h2,h3,h4);
    
done:
    
    if (control != NULL)
    {
        com::xvd::neuron::session::ControlTypeSupport::delete_data(control);
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
    
    return s;
}

bool SCPMaster::DeleteMasterObject(SCPMasterObject* aSession) 
{
    
    DDS_ReturnCode_t retcode;
    com::xvd::neuron::session::Control *control = NULL;
    bool retval = false;
    
    control = com::xvd::neuron::session::ControlTypeSupport::create_data();

    if (control == NULL) 
    {
        //TODO: Error handling
        goto done;
    }
    
    retcode = controlWriter->get_key_value(*control,aSession->GetControlInstanceHandle());
    if (retcode != DDS_RETCODE_OK) 
    {
        // TODO: Error handling
        goto done;
    }

    retcode = controlWriter->dispose(*control,aSession->GetControlInstanceHandle());
    if (retcode != DDS_RETCODE_OK) 
    {  
        // TODO: Error handling
        goto done;
    }
    
    delete aSession;
    retval = true;
    
done:
    
    if (control != NULL) 
    {
        com::xvd::neuron::session::ControlTypeSupport::delete_data(control);
    }
    
    return retval;
}

bool SCPMaster::Send(com::xvd::neuron::session::Control *c,DDS_InstanceHandle_t ih) 
{
    DDS_ReturnCode_t retcode;
    retcode = controlWriter->write(*c, ih);
    if (retcode != DDS_RETCODE_OK)
    {
        // TODO: Error handling
        return false;
    }
    return true;
}

bool SCPMaster::GetMasterObjectState(DDS_InstanceHandle_t instance,com::xvd::neuron::session::State *state)
{
    com::xvd::neuron::session::StateSeq data_seq;
    DDS_SampleInfoSeq info_seq;
    DDS_ReturnCode_t retcode;
    
    data_seq.maximum(1);
    data_seq.length(1);
    retcode = stateReader->read_instance(data_seq,info_seq,
                                         1,
                                         instance,
                                         DDS_READ_SAMPLE_STATE,
                                         DDS_NOT_NEW_VIEW_STATE,
                                         DDS_ALIVE_INSTANCE_STATE);
    if ((retcode != DDS_RETCODE_OK) && (retcode != DDS_RETCODE_NO_DATA)) 
    {
        //TODO: Error handling
        return false;
    }
    
    if (retcode == DDS_RETCODE_NO_DATA)
    {
        //TODO: Error handling
        return false;
    }
    
    if ((data_seq.length() != 1) || !info_seq[0].valid_data)
    {
        //TODO: Error handling
        return false;
    }
    
    retcode = com::xvd::neuron::session::StateTypeSupport::copy_data(state,&data_seq[0]);
    if (retcode != DDS_RETCODE_OK)
    {
        //TODO: Error handling
        return false;
    }
    
    return true;
}

bool SCPMaster::GetMasterObjectEvents(DDS_InstanceHandle_t instance,
                                      com::xvd::neuron::session::EventSeq *eventSeq)
{
    com::xvd::neuron::session::EventSeq result_seq;
    DDS_SampleInfoSeq info_seq;
    DDS_ReturnCode_t retcode;
    
    retcode = eventReader->read_instance(result_seq,
                                         info_seq,
                                         DDS_LENGTH_UNLIMITED,
                                         instance,
                                         DDS_READ_SAMPLE_STATE,
                                         DDS_NOT_NEW_VIEW_STATE,
                                         DDS_ALIVE_INSTANCE_STATE);
    
    if ((retcode != DDS_RETCODE_OK) && (retcode != DDS_RETCODE_NO_DATA)) 
    {
        //TODO: Error handling
        return false;
    }
    
    if (retcode == DDS_RETCODE_NO_DATA)
    {
        //TODO: Error handling
        return false;
    }
    
    *eventSeq = result_seq;
    
    return true;
}

bool SCPMaster::GetMasterObjectMetrics(DDS_InstanceHandle_t instance,
                                       com::xvd::neuron::session::MetricsSeq *metricsSeq)
{
    com::xvd::neuron::session::MetricsSeq result_seq;
    DDS_SampleInfoSeq info_seq;
    DDS_ReturnCode_t retcode;
    
    retcode = metricsReader->read_instance(result_seq,
                                           info_seq,
                                           DDS_LENGTH_UNLIMITED,
                                           instance,
                                           DDS_READ_SAMPLE_STATE,
                                           DDS_NOT_NEW_VIEW_STATE,
                                           DDS_ALIVE_INSTANCE_STATE);
    
    if ((retcode != DDS_RETCODE_OK) && (retcode != DDS_RETCODE_NO_DATA)) 
    {
        //TODO: Error handling
        return false;
    }
    
    if (retcode == DDS_RETCODE_NO_DATA)
    {
        //TODO: Error handling
        return false;
    }
    
    *metricsSeq = result_seq;
    
    return true;    
}

bool SCPMaster::PostEvent(Event *ev)
{
    upper->SignalEvent(ev);
    return true;
}

