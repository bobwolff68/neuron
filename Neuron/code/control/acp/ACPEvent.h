//!
//! \file ACPEvent.h
//!
//! \brief Defintion of Admin Control Plane (ACP) events
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#ifndef ACP_EVENT_H_
#define ACP_EVENT_H_

#include "neuroncommon.h"
#include "ACPInterface.h"
#include "ACPInterfaceSupport.h"
#include "ACPObject.h"

#define ACP_EVENT_BASE                      0x00000400
#define ACP_EVENT_NEW_SESSION               (ACP_EVENT_BASE+0)
#define ACP_EVENT_UPDATE_SESSION            (ACP_EVENT_BASE+1)
#define ACP_EVENT_DELETE_SESSION            (ACP_EVENT_BASE+2)
#define ACP_EVENT_SESSION_STATE_UPDATE      (ACP_EVENT_BASE+3)
#define ACP_EVENT_SESSION_EVENT             (ACP_EVENT_BASE+4)
#define ACP_EVENT_SESSION_METRICS_UPDATE    (ACP_EVENT_BASE+5)

//! \class ACPEventNewSession
//!
//! \brief New session detected on the ACP.
//!
//! Details: A ACPSlave attachment to the ACP detects new sessions using
//!          DDS key life-cycle management. In case a new instance is detected
//!          a new session created by the ACPSlave object and posted to the
//!          upper layer protocol
//!
class ACPEventNewSession : public Event {
    
public:
    ACPEventNewSession(ACPObject *_session,com::xvd::neuron::acp::Control *_data);
    ACPObject* GetSession();
    com::xvd::neuron::acp::Control *GetData();
    ~ACPEventNewSession();

private:
    ACPObject *session;
    com::xvd::neuron::acp::Control *data;
};

class ACPEventDeleteSession : public Event {
public:
    ACPEventDeleteSession(int _sessionId);
    int GetSessionId();
    ~ACPEventDeleteSession();
    
private:
    int sessionId;
};

//! \class ACPEventT
//!
//! \brief Template for Session Update, State and Metrics events
//!
//! Details: This template defines events for a session update (seen instance),
//!          state changes and metrics updates. Each event carries the IDL
//!          for the event
//!
template<class T,typename TypeSupport,int E>
class ACPEventT : public Event {
public:
    ACPEventT(T *_d) : Event(E)
    {
        d = TypeSupport::create_data();
        TypeSupport::copy_data(d,_d);
    }
    
    T* GetData()
    {
        return d;
    }
    
    ~ACPEventT()
    {
        TypeSupport::delete_data(d);
    }
    
private:
    T *d;
};

//! \class ACPEventUpdateSession
//!
//! \brief ACP session update event. This event is generated for not new 
//!        instances. The payload
typedef ACPEventT<com::xvd::neuron::acp::Control,
                  com::xvd::neuron::acp::ControlTypeSupport,
                  ACP_EVENT_UPDATE_SESSION> ACPEventUpdateSession;

//! \class ACPEventSessionStateUpdate
//!
//! \brief ACP session state event. This event is generated when a state
//!        udpate is detected
typedef ACPEventT<com::xvd::neuron::acp::State,
                  com::xvd::neuron::acp::StateTypeSupport,
                  ACP_EVENT_SESSION_STATE_UPDATE> ACPEventSessionStateUpdate;

//! \class ACPEventSessionEvent
//!
//! \brief ACP session event event. This event is generated when a new event
//!        on a sesssion is detected
typedef ACPEventT<com::xvd::neuron::acp::Event,
                  com::xvd::neuron::acp::EventTypeSupport,
                  ACP_EVENT_SESSION_EVENT> ACPEventSessionEvent;

//! \class ACPEventSessionMetricsUpdate
//!
//! \brief ACP session metrics event. This event is generated when a new metrics
//!        on a sesssion is detected
typedef ACPEventT<com::xvd::neuron::acp::Metrics,
                  com::xvd::neuron::acp::MetricsTypeSupport,
                  ACP_EVENT_SESSION_METRICS_UPDATE> ACPEventSessionMetricsUpdate;
#endif
