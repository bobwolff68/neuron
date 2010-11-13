#include "ndds_cpp.h"
#include "SCPEvent.h"
#include "SCPMasterObject.h"
#include "SCPMaster.h"

SCPMaster::SCPMaster(EventHandler *q,int _srcId, int domainId, const char *qosProfile) : 
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
com::xvd::neuron::scp::MetricsTypeSupport>(q,_srcId,domainId,_srcId,qosProfile)
{
    DDS_ReturnCode_t retcode;
    
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
    
    control->srcId = srcId;
    control->sessionId = sid;
    h1 = controlWriter->register_instance(*control);
    if (DDS_InstanceHandle_is_nil(&h1)) 
    {
        //TODO: Error handling
        goto done;
    }
    
    s = new SCPMasterObject(this,srcId,sid,h1,h2,h3,h4);
    
done:
        
    return s;
}

bool SCPMaster::DeleteMasterObject(SCPMasterObject* aSession) 
{
    
    DDS_ReturnCode_t retcode;
    bool retval = false;
    
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
    
    return retval;
}

DDS_InstanceHandle_t SCPMaster::GetMasterObjectStateHandle(int dstId,int sid)
{
    DDS_InstanceHandle_t ih = DDS_HANDLE_NIL;
    
    state->srcId = dstId;
    state->sessionId = sid;
    ih = stateReader->lookup_instance(*state);
    
    return ih;
}

DDS_InstanceHandle_t SCPMaster::GetMasterObjectEventHandle(int dstId, int sid)
{
    DDS_InstanceHandle_t ih = DDS_HANDLE_NIL;
    
    event->srcId = dstId;
    event->sessionId = sid;
    
    ih = eventReader->lookup_instance(*event);
    
    return ih;
}

DDS_InstanceHandle_t SCPMaster::GetMasterObjectMetricsHandle(int dstId,int sid)
{
    DDS_InstanceHandle_t ih = DDS_HANDLE_NIL;
    
    metrics->srcId = dstId;
    metrics->sessionId = sid;
    
    ih = metricsReader->lookup_instance(*metrics);
    
    return ih;
}


