//!
//! \file ACPMasterObject.h
//!
//! \brief Defintion of Session Control Plane (ACP) Master object
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#ifndef ACP_MASTER_OBJECT_H_
#define ACP_MASTER_OBJECT_H_

#include "ACPObject.h"

class ACPMaster;
//!
//! \class ACPMasterObject
//!
//! \brief The ACPMasterObject represents a unique session on the ACP.
//!
//! Details:
//!
//! \todo 
//!
class ACPMasterObject : public ACPObject {
public:
    
    //! Constructor a ACPMasterObject
    //!
    //! \param[in] sm         Owner        
    //! \param[in]            srcId
    //! \param[in] instance Instance handle
    ACPMasterObject(ACPMaster *_sm,int sfId,int _sessionId,
                    DDS_InstanceHandle_t,
                    DDS_InstanceHandle_t _stateH,
                    DDS_InstanceHandle_t _eventH,
                    DDS_InstanceHandle_t _metricsH);

    //! Deconstructor a ACPMasterObject
    //!
    ~ACPMasterObject();
    
    //! Create/Update a session
    //!
    //! \param[in] sessionId   Session ID    
    bool Send(com::xvd::neuron::acp::Control *control,int);
    
    //! Get the current state of the session
    com::xvd::neuron::acp::State* GetState(int dstId);

    //! Get the current events for the session
    com::xvd::neuron::acp::EventSeq* GetEvents(int dstId);
    
    //! Get the current metrics for the session
    com::xvd::neuron::acp::MetricsSeq* GetMetrics(int dstId);
        
    //! Internal function
    DDS_InstanceHandle_t GetControlInstanceHandle();

private:
    //! \var sm 
    //! \brief The ACPMaster that owns this object    
    ACPMaster *sm;

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
    com::xvd::neuron::acp::State* state;
    
    //! \var event 
    //! \brief contains current events
    com::xvd::neuron::acp::EventSeq eventSeq;
    
    //! \var metrics
    //! \brief contains current metrics
    com::xvd::neuron::acp::MetricsSeq metricsSeq;
};

#endif
