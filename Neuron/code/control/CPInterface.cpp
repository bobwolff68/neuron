//!
//! \file CPInterface.h
//!
//! \brief Defintion of Control Plane DDS evens
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
//! \todo Remove the logging function
//!
#include "CPInterface.h"

//! \brief Log a info string
//!
//! \param [in] logfmt - Formatting string
//! \param [in] ...    - Log arguments
//!
//! \todo Replace this function with a real logging function
void ControlLogInfo(const char *logfmt,...)
{
    va_list vl;
    va_start(vl,logfmt);
    printf("ControlLog[INFO]:");
    vprintf(logfmt,vl);
    va_end(vl);
}

//! \brief Log a warning string
//!
//! \param [in] logfmt - Formatting string
//! \param [in] ...    - Log arguments
//!
//! \todo Replace this function with a real logging function
void ControlLogWarning(const char *logfmt,...)
{
    va_list vl;
    va_start(vl,logfmt);
    printf("ControlLog[WARNING]:");
    vprintf(logfmt,vl);
    va_end(vl);
}

//! \brief Log an Error string
//!
//! \param [in] logfmt - Formatting string
//! \param [in] ...    - Log arguments
//!
//! \todo Replace this function with a real logging function
void ControlLogError(const char *logfmt,...)
{
    va_list vl;
    va_start(vl,logfmt);
    printf("ControlLog[ERROR]:");
    vprintf(logfmt,vl);
    va_end(vl);
}    

//! \brief Create Control Plane DataReader listener
//!
//! \param [in] _cp - Control Interface class
//!
CPDataReaderListener::CPDataReaderListener(CPInterface *_cp)
{
    cp = _cp;
}

//! \brief Post deadline missed event
//!
//! \param [in] reader - reader event occured on
//! \param [in] status - deadline missed status
//!
void CPDataReaderListener::on_requested_deadline_missed(DDSDataReader *reader,
                            const DDS_RequestedDeadlineMissedStatus& status)
{
    cp->PostEvent(new DDSEventRequestedDeadlineMissed(reader,&status));
}

//! \brief Post requested incompatible qos event
//!
//! \param [in] reader - reader event occured on
//! \param [in] status - incompatible qos status
//!
void CPDataReaderListener::on_requested_incompatible_qos(DDSDataReader *reader,
                            const DDS_RequestedIncompatibleQosStatus& status)
{
    cp->PostEvent(new DDSEventRequestedIncompatibleQos(reader,&status));
}

//! \brief Post sample rejected event
//!
//! \param [in] reader - reader event occured on
//! \param [in] status - sample rejected status
//!
void CPDataReaderListener::on_sample_rejected(DDSDataReader *reader,
                            const DDS_SampleRejectedStatus& status)
{
    cp->PostEvent(new DDSEventSampleRejected(reader,&status));
}

//! \brief Post liveliness changed event
//!
//! \param [in] reader - reader event occured on
//! \param [in] status - liveliness changed status
//!
void CPDataReaderListener::on_liveliness_changed(DDSDataReader *reader,
                            const DDS_LivelinessChangedStatus& status)
{
    cp->PostEvent(new DDSEventLivelinessChanged(reader,&status));
}

//! \brief Post sample lost event
//!
//! \param [in] reader - reader event occured on
//! \param [in] status - sample lost status
//!
void CPDataReaderListener::on_sample_lost(DDSDataReader *reader,
                            const DDS_SampleLostStatus& status)
{
    cp->PostEvent(new DDSEventSampleLostStatus(reader,&status));    
}

//! \brief Post subscription matched event
//!
//! \param [in] reader - reader event occured on
//! \param [in] status - subscription matched status
//!
void CPDataReaderListener::on_subscription_matched(DDSDataReader *reader,
                            const DDS_SubscriptionMatchedStatus& status)
{
    cp->PostEvent(new DDSEventSubscriptionMatched(reader,&status));    
}

//! \brief Create Control Plane DataWriter listener
//!
//! \param [in] _cp - Control Interface class
//!
CPDataWriterListener::CPDataWriterListener(CPInterface *_cp)
{
    cp = _cp;
}

//! \brief Post offered deadline missed event
//!
//! \param [in] reader - writer event occured on
//! \param [in] status - offered deadline missed status
//!
void CPDataWriterListener::on_offered_deadline_missed(DDSDataWriter *writer,
                                const DDS_OfferedDeadlineMissedStatus& status)
{
    cp->PostEvent(new DDSEventOfferedDeadlineMissed(writer,&status));    
}

//! \brief Post liveliness lost event
//!
//! \param [in] reader - writer event occured on
//! \param [in] status - liveliness lost status
//!
void CPDataWriterListener::on_liveliness_lost(DDSDataWriter *writer, 
                        const DDS_LivelinessLostStatus &status)
{
    cp->PostEvent(new DDSEventLivelinessLost(writer,&status));    
}

//! \brief Post incompatible qos event
//!
//! \param [in] reader - writer event occured on
//! \param [in] status - liveliness lost status
//!
void CPDataWriterListener::on_offered_incompatible_qos(DDSDataWriter *writer, 
                                 const DDS_OfferedIncompatibleQosStatus &status)
{
    cp->PostEvent(new DDSEventOfferedIncompatibleQos(writer,&status));    
}

//! \brief Post publication matched event
//!
//! \param [in] reader - writer event occured on
//! \param [in] status - publication matched status
//!
void CPDataWriterListener::on_publication_matched(DDSDataWriter *writer, 
                            const DDS_PublicationMatchedStatus &status)
{
    cp->PostEvent(new DDSEventPublicationMatched(writer,&status));
}

//! \brief Post reliable writer cache changed event
//!
//! \param [in] reader - writer event occured on
//! \param [in] status - reliable cache status
//!
void CPDataWriterListener::on_reliable_writer_cache_changed(DDSDataWriter *writer, 
                            const DDS_ReliableWriterCacheChangedStatus &status)
{
    cp->PostEvent(new DDSEventReliableWriterCacheChanged(writer,&status));
}

//! \brief Post reliable reader activity changed event
//!
//! \param [in] reader - writer event occured on
//! \param [in] status - reliable reader status
//!
void CPDataWriterListener::on_reliable_reader_activity_changed(DDSDataWriter *writer, 
                            const DDS_ReliableReaderActivityChangedStatus &status)
{
    cp->PostEvent(new DDSEventReliableReaderActivityChanged(writer,&status));
}
