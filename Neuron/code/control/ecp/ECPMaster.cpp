#include "ndds_cpp.h"
#include "ECPEvent.h"
#include "ECPMasterObject.h"
#include "ECPMaster.h"

ECPMaster::ECPMaster(EventHandler *q,int _srcId, int domainId, const char *qosProfile) : 
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
com::xvd::neuron::ecp::MetricsTypeSupport>(q,_srcId,domainId,_srcId,qosProfile)
{
    DDS_ReturnCode_t retcode;
    
    ECPMasterMetricsReaderListener *metricsListener = new ECPMasterMetricsReaderListener(this,metricsReader);
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
    ECPMasterEventReaderListener *eventListener = new ECPMasterEventReaderListener(this,eventReader);
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
    ECPMasterStateReaderListener *stateListener = new ECPMasterStateReaderListener(this,stateReader);
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
        //TODO: Error handling
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



