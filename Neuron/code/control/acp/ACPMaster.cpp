#include "ndds_cpp.h"
#include "ACPEvent.h"
#include "ACPMasterObject.h"
#include "ACPMaster.h"

ACPMaster::ACPMaster(EventHandler *q,int _srcId, int domainId, const char *qosProfile) : 
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
com::xvd::neuron::acp::MetricsTypeSupport>(q,_srcId,domainId,_srcId,qosProfile)
{
    DDS_ReturnCode_t retcode;
    
    ACPMasterMetricsReaderListener *metricsListener = new ACPMasterMetricsReaderListener(this,metricsReader);
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
    ACPMasterEventReaderListener *eventListener = new ACPMasterEventReaderListener(this,eventReader);
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
    ACPMasterStateReaderListener *stateListener = new ACPMasterStateReaderListener(this,stateReader);
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
        //TODO: Error handling
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



