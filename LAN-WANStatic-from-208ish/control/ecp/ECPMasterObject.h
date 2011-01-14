//!
//! \file ECPMasterObject.h
//!
//! \brief Defintion of Session Control Plane (ECP) Master object
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#ifndef ECP_MASTER_OBJECT_H_
#define ECP_MASTER_OBJECT_H_

#include "ECPObject.h"

class ECPMaster;
//!
//! \class ECPMasterObject
//!
//! \brief The ECPMasterObject represents a unique session on the ECP.
//!
//! Details:
//!
//! \todo 
//!
class ECPMasterObject : public ECPObject {
public:
    
    //! Constructor a ECPMasterObject
    //!
    //! \param[in] sm         Owner        
    //! \param[in]            srcId
    //! \param[in] instance Instance handle
    ECPMasterObject(ECPMaster *_sm,int sfId,int _sessionId,
                    DDS_InstanceHandle_t,
                    DDS_InstanceHandle_t _stateH,
                    DDS_InstanceHandle_t _eventH,
                    DDS_InstanceHandle_t _metricsH);

    //! Deconstructor a ECPMasterObject
    //!
    ~ECPMasterObject();
    
    //! Create/Update a session
    //!
    //! \param[in] sessionId   Session ID    
    bool Send(com::xvd::neuron::ecp::Control *control,int);
    
    //! Get the current state of the session
    com::xvd::neuron::ecp::State* GetState(int dstId);

    //! Get the current events for the session
    com::xvd::neuron::ecp::EventSeq* GetEvents(int dstId);
    
    //! Get the current metrics for the session
    com::xvd::neuron::ecp::MetricsSeq* GetMetrics(int dstId);
        
    //! Internal function
    DDS_InstanceHandle_t GetControlInstanceHandle();

private:
    //! \var sm 
    //! \brief The ECPMaster that owns this object    
    ECPMaster *sm;

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
    com::xvd::neuron::ecp::State* state;
    
    //! \var event 
    //! \brief contains current events
    com::xvd::neuron::ecp::EventSeq eventSeq;
    
    //! \var metrics
    //! \brief contains current metrics
    com::xvd::neuron::ecp::MetricsSeq metricsSeq;
};

#endif
