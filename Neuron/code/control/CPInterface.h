//!
//! \file CPInterfaceT.h
//!
//! \brief Defintion of Control Plane (CP)
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#ifndef CPINTERFACE_H_
#define CPINTERFACE_H_

#include "ndds_cpp.h"
#include "neuroncommon.h"

// Empty base-class for a CPObject
class CPObject {
};

// Empty base-class for a CP Master/Slave object
class CPInterface 
{
public:
    virtual bool PostEvent(Event*) = 0;
 
public:
    
};

void ControlLogInfo(const char *logfmt,...);

void ControlLogWarning(const char *logfmt,...);

void ControlLogError(const char *logfmt,...);

class CPDataReaderListener :  public DDSDataReaderListener
{
public:
    CPDataReaderListener(CPInterface *_cp);
    
    virtual void on_requested_deadline_missed(DDSDataReader *reader,
                                    const DDS_RequestedDeadlineMissedStatus& status);
    
    virtual void on_requested_incompatible_qos(DDSDataReader *reader,
                                    const DDS_RequestedIncompatibleQosStatus& status);
    
    virtual void on_sample_rejected(DDSDataReader *reader,
                                    const DDS_SampleRejectedStatus& status);
    
    virtual void on_liveliness_changed(DDSDataReader *reader,
                                    const DDS_LivelinessChangedStatus& status);
    
    virtual void on_sample_lost(DDSDataReader *reader,
                                    const DDS_SampleLostStatus& status);
    
    virtual void on_subscription_matched(DDSDataReader *reader,
                                    const DDS_SubscriptionMatchedStatus& status);
private:
    CPInterface *cp;
};

class CPDataWriterListener : public DDSDataWriterListener 
{
public:
    CPDataWriterListener(CPInterface *_cp);
    
    virtual void on_offered_deadline_missed(DDSDataWriter *writer,
                                    const DDS_OfferedDeadlineMissedStatus& status);
        
    virtual void on_liveliness_lost (DDSDataWriter *writer, 
                                    const DDS_LivelinessLostStatus &status);

    virtual void on_offered_incompatible_qos (DDSDataWriter *writer, 
                                    const DDS_OfferedIncompatibleQosStatus &status);
    
    virtual void on_publication_matched(DDSDataWriter *writer, 
                                    const DDS_PublicationMatchedStatus &status);
    
    virtual void on_reliable_writer_cache_changed (DDSDataWriter *writer, 
                                    const DDS_ReliableWriterCacheChangedStatus &status);
    
    virtual void on_reliable_reader_activity_changed (DDSDataWriter *writer, 
                                    const DDS_ReliableReaderActivityChangedStatus &status);
    
private:
    CPInterface *cp;
};
#endif
