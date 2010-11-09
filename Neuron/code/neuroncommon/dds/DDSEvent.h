#ifndef DDS_EVENT_H_
#define DDS_EVENT_H_

#include "neuroncommon.h"
#include "ndds_cpp.h"

#define DDS_EVENT_BASE                             0x00000200
#define DDS_EVENT_REQUESTED_DEADLINE_MISSED        (DDS_EVENT_BASE+0)
#define DDS_EVENT_LIVELINESS_CHANGED               (DDS_EVENT_BASE+1)
#define DDS_EVENT_REQUSTED_INCOMPATIBLE_QOS        (DDS_EVENT_BASE+2)
#define DDS_EVENT_SAMPLE_REJECTED                  (DDS_EVENT_BASE+3)    
#define DDS_EVENT_SAMPLE_LOST                      (DDS_EVENT_BASE+4) 
#define DDS_EVENT_SUBSCRIPTION_MATCHED             (DDS_EVENT_BASE+5)
#define DDS_EVENT_OFFERED_DEADLINE_MISSED          (DDS_EVENT_BASE+6)
#define DDS_EVENT_LIVELINESS_LOST                  (DDS_EVENT_BASE+7)
#define DDS_EVENT_OFFERED_INCOMPATIBLE_QOS         (DDS_EVENT_BASE+8)
#define DDS_EVENT_PUBLICATION_MATCHED              (DDS_EVENT_BASE+9)
#define DDS_EVENT_RELIABLE_WRITER_CACHE_CHANGED    (DDS_EVENT_BASE+10)
#define DDS_EVENT_RELIABLE_READER_ACTIVITY_CHANGED (DDS_EVENT_BASE+11) 

template <typename T,int E>
class DDSEvent : public Event 
{
public:
    DDSEvent(const T *_t) : Event(E) 
    {
        t = *_t;
    }
    
    T GetEventData() 
    {
        return t;
    }
    
private:
    T t;
};

typedef class DDSEvent<DDS_RequestedDeadlineMissedStatus,
                       DDS_EVENT_REQUESTED_DEADLINE_MISSED> 
                       DDSEventRequestedDeadlineMissed;

typedef class DDSEvent<DDS_LivelinessChangedStatus,
                       DDS_EVENT_LIVELINESS_CHANGED> 
                       DDSEventLivelinessChanged;

typedef class DDSEvent<DDS_RequestedIncompatibleQosStatus,
                       DDS_EVENT_REQUSTED_INCOMPATIBLE_QOS> 
                       DDSEventRequestedIncompatibleQos;

typedef class DDSEvent<DDS_SampleRejectedStatus,
                       DDS_EVENT_SAMPLE_REJECTED> 
                       DDSEventSampleRejected;

typedef class DDSEvent<DDS_SampleLostStatus,
                       DDS_EVENT_SAMPLE_LOST> 
                       DDSEventSampleLostStatus;

typedef class DDSEvent<DDS_SubscriptionMatchedStatus,
                       DDS_EVENT_SUBSCRIPTION_MATCHED> 
                       DDSEventSubscriptionMatched;

typedef class DDSEvent<DDS_OfferedDeadlineMissedStatus,
                       DDS_EVENT_OFFERED_DEADLINE_MISSED> 
                       DDSEventOfferedDeadlineMissed;

typedef class DDSEvent<DDS_LivelinessLostStatus,
                       DDS_EVENT_LIVELINESS_LOST> 
                       DDSEventLivelinessLost;

typedef class DDSEvent<DDS_OfferedIncompatibleQosStatus,
                       DDS_EVENT_OFFERED_INCOMPATIBLE_QOS> 
                       DDSEventOfferedIncompatibleQos;

typedef class DDSEvent<DDS_PublicationMatchedStatus,
                       DDS_EVENT_PUBLICATION_MATCHED> 
                       DDSEventPublicationMatched;

typedef class DDSEvent<DDS_ReliableWriterCacheChangedStatus,
                       DDS_EVENT_RELIABLE_WRITER_CACHE_CHANGED> 
                       DDSEventReliableWriterCacheChanged;

typedef class DDSEvent<DDS_ReliableReaderActivityChangedStatus,
                       DDS_EVENT_RELIABLE_READER_ACTIVITY_CHANGED>
                       DDSEventReliableReaderActivityChanged;


#endif
