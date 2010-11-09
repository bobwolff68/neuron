#include "ndds_cpp.h"
#include "SCPEvent.h"
#include "SCPMasterObject.h"
#include "SCPMaster.h"

using namespace std;

class SCPMasterReaderListener : public DDSDataReaderListener 
{
public:
    virtual void on_requested_deadline_missed(
                    DDSDataReader* /*reader*/,
                    const DDS_RequestedDeadlineMissedStatus& /*status*/) {}
    
    virtual void on_requested_incompatible_qos(
                    DDSDataReader* /*reader*/,
                    const DDS_RequestedIncompatibleQosStatus& /*status*/) {}
    
    virtual void on_sample_rejected(
                    DDSDataReader* /*reader*/,
                    const DDS_SampleRejectedStatus& /*status*/) {}
    
    virtual void on_liveliness_changed(
                    DDSDataReader* /*reader*/,
                    const DDS_LivelinessChangedStatus& /*status*/) {}
    
    virtual void on_sample_lost(
                    DDSDataReader* /*reader*/,
                    const DDS_SampleLostStatus& /*status*/) {}
    
    virtual void on_subscription_matched(
                    DDSDataReader* /*reader*/,
                    const DDS_SubscriptionMatchedStatus& /*status*/) {}
};

template<class DataSeq, class Reader,class EventKind>
class SCPMasterReaderListenerT : public SCPMasterReaderListener
{
public:
    SCPMasterReaderListenerT(SCPMaster *_sm,Reader *reader)
    {
        m_reader = reader;
        sm = _sm;
    }
    
    void on_data_available(DDSDataReader* reader)
    {
        DataSeq data_seq;
        DDS_SampleInfoSeq info_seq;
        DDS_ReturnCode_t retcode;
        int i;
        Event *ev;
        
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
                    break;
                case DDS_NOT_NEW_VIEW_STATE:
                    if (!info_seq[i].valid_data) {
                        // TODO: Error logging
                    } else {
                    }
                    break;
                default:
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
                    }
                    break;
                default:
                    break;
            }
            
            if (info_seq[i].valid_data) 
            {
                ev = new EventKind(&data_seq[i]);
                sm->PostEvent(ev);
            }
        }
        
        retcode = m_reader->return_loan(data_seq, info_seq);
        if (retcode != DDS_RETCODE_OK) {
            // TODO: Error logging
        }
    };
    
private:
    Reader *m_reader;
    SCPMaster *sm;    
};

typedef SCPMasterReaderListenerT<com::xvd::neuron::session::StateSeq,
com::xvd::neuron::session::StateDataReader,
SCPEventSessionStateUpdate> SCPMasterStateReaderListener;

typedef SCPMasterReaderListenerT<com::xvd::neuron::session::EventSeq,
com::xvd::neuron::session::EventDataReader,
SCPEventSessionEvent> SCPMasterEventReaderListener;

typedef SCPMasterReaderListenerT<com::xvd::neuron::session::MetricsSeq,
com::xvd::neuron::session::MetricsDataReader,
SCPEventSessionMetricsUpdate> SCPMasterMetricsReaderListener;

SCPMaster::SCPMaster(EventHandler *q,int _srcId, int domainId, const char *qosProfile) : 
CPMasterT<com::xvd::neuron::session::ControlTypeSupport,
com::xvd::neuron::session::EventTypeSupport,
com::xvd::neuron::session::StateTypeSupport,
com::xvd::neuron::session::MetricsTypeSupport>(domainId,_srcId,qosProfile)
{
    controlWriter = com::xvd::neuron::session::ControlDataWriter::narrow(m_controlWriter);
    if (controlWriter == NULL)
    {
        return;
    }
    
    stateReader = com::xvd::neuron::session::StateDataReader::narrow(m_stateReader);
    if (stateReader == NULL) 
    {
        return;
    }
    
    SCPMasterStateReaderListener *stateListener = new SCPMasterStateReaderListener(this,stateReader);
    m_stateReader->set_listener(stateListener, DDS_STATUS_MASK_ALL);
    
    eventReader = com::xvd::neuron::session::EventDataReader::narrow(m_eventReader);
    if (eventReader == NULL) 
    {
        return;
    }
    
    SCPMasterEventReaderListener *eventListener = new SCPMasterEventReaderListener(this,eventReader);
    m_eventReader->set_listener(eventListener, DDS_STATUS_MASK_ALL);
    
    metricsReader = com::xvd::neuron::session::MetricsDataReader::narrow(m_metricsReader);
    if (metricsReader == NULL) 
    {
        return;
    }  
    
    SCPMasterMetricsReaderListener *metricsListener = new SCPMasterMetricsReaderListener(this,metricsReader);
    m_metricsReader->set_listener(metricsListener, DDS_STATUS_MASK_ALL);
    
    srcId = _srcId;
    upper = q;
}

SCPMasterObject* SCPMaster::CreateMasterObject(int sid)
{
    SCPMasterObject *s;
    DDS_InstanceHandle_t h1 = DDS_HANDLE_NIL;
    com::xvd::neuron::session::Control *control = NULL;
    
    control = com::xvd::neuron::session::ControlTypeSupport::create_data();
    if (control == NULL)
    {
        //TODO: Error log
        goto done;
    }

    control->srcId = srcId;
    h1 = controlWriter->register_instance(*control);
    if (DDS_InstanceHandle_is_nil(&h1)) 
    {
        //TODO: Error log
        goto done;
    }
    
    s = new SCPMasterObject(this,srcId,sid,h1);
    
done:
    if (control != NULL)
    {
        com::xvd::neuron::session::ControlTypeSupport::delete_data(control);
    }
    
    return s;
}

bool SCPMaster::DeleteMasterObject(SCPMasterObject* aSession) 
{
    
    DDS_ReturnCode_t retcode;
    com::xvd::neuron::session::Control *control;

    control = com::xvd::neuron::session::ControlTypeSupport::create_data();

    retcode = controlWriter->get_key_value(*control,aSession->GetControlInstanceHandle());
    if (retcode != DDS_RETCODE_OK) 
    {
	return false;
    }

    com::xvd::neuron::session::ControlTypeSupport::delete_data(control);

    retcode = controlWriter->dispose(*control,
                                     DDS_HANDLE_NIL);
    if (retcode != DDS_RETCODE_OK) 
    {
        return false;
    }
    delete aSession;
    return true;
}

// TODO: Publish the session object on the wire
bool SCPMaster::Send(com::xvd::neuron::session::Control *c) 
{
    DDS_ReturnCode_t retcode;
    retcode = controlWriter->write(*c, DDS_HANDLE_NIL);
    if (retcode != DDS_RETCODE_OK)
    {
        return false;
    }
    return true;
}

bool SCPMaster::GetMasterObjectState(DDS_InstanceHandle_t instance,com::xvd::neuron::session::State*)
{
    DDS_ReturnCode_t retcode;
    //retcode = stateWriter->write(*aSession.GetState(), DDS_HANDLE_NIL);
    if (retcode != DDS_RETCODE_OK)
    {
        return false;
    }
    return true;
}

bool SCPMaster::GetMasterObjectEvents(DDS_InstanceHandle_t instance,com::xvd::neuron::session::EventSeq*)
{
    
    //return sm->GetState(*this);
    return false;
}

bool SCPMaster::GetMasterObjectMetrics(DDS_InstanceHandle_t instance,com::xvd::neuron::session::MetricsSeq*)
{
    //return sm->GetState(*this);
    return false;
}

bool SCPMaster::PostEvent(Event *ev)
{
    upper->SignalEvent(ev);
    return true;
}

