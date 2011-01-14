//!
//! \file LSCPMasterObject.h
//!
//! \brief Defintion of Session Control Plane (LSCP) Master object
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#ifndef LSCP_MASTER_OBJECT_H_
#define LSCP_MASTER_OBJECT_H_

#include "LSCPObject.h"

class LSCPMaster;
//!
//! \class LSCPMasterObject
//!
//! \brief The LSCPMasterObject represents a unique session on the LSCP.
//!
//! Details:
//!
//! \todo 
//!
class LSCPMasterObject : public LSCPObject {
public:
    
    //! Constructor a LSCPMasterObject
    //!
    //! \param[in] sm         Owner        
    //! \param[in]            srcId
    //! \param[in] instance Instance handle
    LSCPMasterObject(LSCPMaster *_sm,int sfId,int _sessionId,
                    DDS_InstanceHandle_t,
                    DDS_InstanceHandle_t _stateH,
                    DDS_InstanceHandle_t _eventH,
                    DDS_InstanceHandle_t _metricsH);

    //! Deconstructor a LSCPMasterObject
    //!
    ~LSCPMasterObject();
    
    //! Create/Update a session
    //!
    //! \param[in] sessionId   Session ID    
    bool Send(com::xvd::neuron::lscp::Control *control,int);
    
    //! Get the current state of the session
    com::xvd::neuron::lscp::State* GetState(int dstId);

    //! Get the current events for the session
    com::xvd::neuron::lscp::EventSeq* GetEvents(int dstId);
    
    //! Get the current metrics for the session
    com::xvd::neuron::lscp::MetricsSeq* GetMetrics(int dstId);
        
    //! Internal function
    DDS_InstanceHandle_t GetControlInstanceHandle();

private:
    //! \var sm 
    //! \brief The LSCPMaster that owns this object    
    LSCPMaster *sm;

    //! \var controlHandle 
    //! \brief instance handle for the control object 
    DDS_InstanceHandle_t controlHandle;

    //! \var stateHandle 
    //! \brief instance handle for the state object
    DDS_InstanceHandle_t stateHandle;
    
    //! \var eventHandle 
    //! \brief instance handle for the event object
    DDS_InstanceHandle_t eventHandle;

    //! \var metricsHandle 
    //! \brief instance handle for the metrics object
    DDS_InstanceHandle_t metricsHandle;

    //! \var state 
    //! \brief contains current state
    com::xvd::neuron::lscp::State* state;
    
    //! \var event 
    //! \brief contains current events
    com::xvd::neuron::lscp::EventSeq eventSeq;
    
    //! \var metrics
    //! \brief contains current metrics
    com::xvd::neuron::lscp::MetricsSeq metricsSeq;
};

#endif
