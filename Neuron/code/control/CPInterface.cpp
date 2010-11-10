//!
//! \file CPInterface.h
//!
//! \brief Defintion of Control Plane DDS events
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#include "CPInterface.h"

CPDataReaderListener::CPDataReaderListener(CPInterface *_cp)
{
    cp = _cp;
}

void CPDataReaderListener::on_requested_deadline_missed(DDSDataReader *reader,
                            const DDS_RequestedDeadlineMissedStatus& status)
{
    cp->PostEvent(new DDSEventRequestedDeadlineMissed(reader,&status));
}

void CPDataReaderListener::on_requested_incompatible_qos(DDSDataReader *reader,
                            const DDS_RequestedIncompatibleQosStatus& status)
{
    cp->PostEvent(new DDSEventRequestedIncompatibleQos(reader,&status));
}

void CPDataReaderListener::on_sample_rejected(DDSDataReader *reader,
                            const DDS_SampleRejectedStatus& status)
{
    cp->PostEvent(new DDSEventSampleRejected(reader,&status));
}

void CPDataReaderListener::on_liveliness_changed(DDSDataReader *reader,
                            const DDS_LivelinessChangedStatus& status)
{
    cp->PostEvent(new DDSEventLivelinessChanged(reader,&status));
}


void CPDataReaderListener::on_sample_lost(DDSDataReader *reader,
                            const DDS_SampleLostStatus& status)
{
    cp->PostEvent(new DDSEventSampleLostStatus(reader,&status));    
}

void CPDataReaderListener::on_subscription_matched(DDSDataReader *reader,
                            const DDS_SubscriptionMatchedStatus& status)
{
    cp->PostEvent(new DDSEventSubscriptionMatched(reader,&status));    
}

CPDataWriterListener::CPDataWriterListener(CPInterface *_cp)
{
    cp = _cp;
}


void CPDataWriterListener::on_offered_deadline_missed(DDSDataWriter *writer,
                                const DDS_OfferedDeadlineMissedStatus& status)
{
    cp->PostEvent(new DDSEventOfferedDeadlineMissed(writer,&status));    
}

void CPDataWriterListener::on_liveliness_lost(DDSDataWriter *writer, 
                        const DDS_LivelinessLostStatus &status)
{
    cp->PostEvent(new DDSEventLivelinessLost(writer,&status));    
}

void CPDataWriterListener::on_offered_incompatible_qos(DDSDataWriter *writer, 
                                 const DDS_OfferedIncompatibleQosStatus &status)
{
    cp->PostEvent(new DDSEventOfferedIncompatibleQos(writer,&status));    
}

void CPDataWriterListener::on_publication_matched(DDSDataWriter *writer, 
                            const DDS_PublicationMatchedStatus &status)
{
    cp->PostEvent(new DDSEventPublicationMatched(writer,&status));
}

void CPDataWriterListener::on_reliable_writer_cache_changed(DDSDataWriter *writer, 
                            const DDS_ReliableWriterCacheChangedStatus &status)
{
    cp->PostEvent(new DDSEventReliableWriterCacheChanged(writer,&status));
}

void CPDataWriterListener::on_reliable_reader_activity_changed(DDSDataWriter *writer, 
                            const DDS_ReliableReaderActivityChangedStatus &status)
{
    cp->PostEvent(new DDSEventReliableReaderActivityChanged(writer,&status));
}
