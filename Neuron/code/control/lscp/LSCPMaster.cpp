#include "ndds_cpp.h"
#include "LSCPEvent.h"
#include "LSCPMasterObject.h"
#include "LSCPMaster.h"

LSCPMaster::LSCPMaster(EventHandler *q,int _srcId, int domainId, const char *qosProfile) : 
CPMasterT<
LSCPMasterObject,
com::xvd::neuron::lscp::ControlDataWriter,
com::xvd::neuron::lscp::StateDataReader,
com::xvd::neuron::lscp::EventDataReader,
com::xvd::neuron::lscp::MetricsDataReader,
com::xvd::neuron::lscp::EventSeq,
com::xvd::neuron::lscp::StateSeq,
com::xvd::neuron::lscp::MetricsSeq,
com::xvd::neuron::lscp::Metrics,
com::xvd::neuron::lscp::Event,
com::xvd::neuron::lscp::State,
com::xvd::neuron::lscp::Control,
com::xvd::neuron::lscp::ControlTypeSupport,
com::xvd::neuron::lscp::EventTypeSupport,
com::xvd::neuron::lscp::StateTypeSupport,
com::xvd::neuron::lscp::MetricsTypeSupport>(q,_srcId,domainId,_srcId,qosProfile)
{
    DDS_ReturnCode_t retcode;
    
    LSCPMasterMetricsReaderListener *metricsListener = new LSCPMasterMetricsReaderListener(this,metricsReader);
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
    LSCPMasterEventReaderListener *eventListener = new LSCPMasterEventReaderListener(this,eventReader);
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
    LSCPMasterStateReaderListener *stateListener = new LSCPMasterStateReaderListener(this,stateReader);
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

LSCPMaster::~LSCPMaster()
{
    LSCPMasterEventReaderListener *eventListener = NULL;
    LSCPMasterMetricsReaderListener *metricsListener = NULL;
    LSCPMasterStateReaderListener *stateListener = NULL;

    if (eventReader)
    {
        eventListener = (LSCPMasterEventReaderListener*)eventReader->get_listener();
        eventReader->set_listener(NULL,DDS_STATUS_MASK_NONE);
        if (eventListener != NULL)
        {
            delete eventListener;
        }
    }

    if (stateReader)
    {
        stateListener = (LSCPMasterStateReaderListener*)stateReader->get_listener();
        stateReader->set_listener(NULL,DDS_STATUS_MASK_NONE);
        if (stateListener != NULL)
        {
            delete stateListener;
        }
    }

    if (metricsReader)
    {
        metricsListener = (LSCPMasterMetricsReaderListener*)metricsReader->get_listener();
        metricsReader->set_listener(NULL,DDS_STATUS_MASK_NONE);
        if (metricsListener != NULL)
        {
            delete metricsListener;
        }
    }
}

LSCPMasterObject* LSCPMaster::CreateMasterObject(int sid)
{
    LSCPMasterObject *s = NULL;
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
    
    s = new LSCPMasterObject(this,srcId,sid,h1,h2,h3,h4);
    
done:
        
    return s;
}

bool LSCPMaster::DeleteMasterObject(LSCPMasterObject* aSession) 
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



