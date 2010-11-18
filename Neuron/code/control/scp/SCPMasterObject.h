//!
//! \file SCPMasterObject.h
//!
//! \brief Defintion of Session Control Plane (SCP) Master object
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#ifndef SCP_MASTER_OBJECT_H_
#define SCP_MASTER_OBJECT_H_

#include "SCPObject.h"

class SCPMaster;
//!
//! \class SCPMasterObject
//!
//! \brief The SCPMasterObject represents a unique scp on the SCP.
//!
//! Details:
//!
//! \todo 
//!
class SCPMasterObject : public SCPObject {
public:
    
    //! \brief Constructor a SCPMasterObject
    //!
    //! \param[in] sm       - Owner        
    //! \param[in]          - srcId
    //! \param[in] instance - Instance handle
    SCPMasterObject(SCPMaster *_sm,int sfId,int _scpId,
                    DDS_InstanceHandle_t,
                    DDS_InstanceHandle_t _stateH,
                    DDS_InstanceHandle_t _eventH,
                    DDS_InstanceHandle_t _metricsH);

    //! \brief Deconstructor a SCPMasterObject
    //!
    ~SCPMasterObject();
    
    //! \brief Create/Update a scp
    //!
    //! \param[in] control - Session ID    
    //! \param[in] dstId   - destination ID    
    bool Send(com::xvd::neuron::scp::Control *control,int dstId);
    
    //! \brief Get the current state of the scp
    //! \param[in] dstId - destination ID    
    com::xvd::neuron::scp::State* GetState(int dstId);

    //! \brief Get the current events for the scp
    //! \param[in] dstId - destination ID    
    com::xvd::neuron::scp::EventSeq* GetEvents(int dstId);
    
    //! \brief Get the current metrics for the scp
    //! \param[in] dstId - destination ID    
    com::xvd::neuron::scp::MetricsSeq* GetMetrics(int dstId);
        
    //! \brief Internal function
    DDS_InstanceHandle_t GetControlInstanceHandle();

private:
    //! \var sm 
    //! \brief The SCPMaster that owns this object    
    SCPMaster *sm;

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
    com::xvd::neuron::scp::State* state;
    
    //! \var event 
    //! \brief contains current events
    com::xvd::neuron::scp::EventSeq eventSeq;
    
    //! \var metrics
    //! \brief contains current metrics
    com::xvd::neuron::scp::MetricsSeq metricsSeq;
};

#endif
