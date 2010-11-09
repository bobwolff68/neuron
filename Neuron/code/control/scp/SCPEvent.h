//!
//! \file SCPEvent.h
//!
//! \brief Defintion of Session Control Plane (SCP) events
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#ifndef SCP_EVENT_H_
#define SCP_EVENT_H_

#include "neuroncommon.h"
#include "SCPInterface.h"
#include "SCPInterfaceSupport.h"
#include "SCPObject.h"

#define SCP_EVENT_BASE                      0x00000100
#define SCP_EVENT_NEW_SESSION               (SCP_EVENT_BASE+0)
#define SCP_EVENT_UPDATE_SESSION            (SCP_EVENT_BASE+1)
#define SCP_EVENT_DELETE_SESSION            (SCP_EVENT_BASE+2)
#define SCP_EVENT_SESSION_STATE_UPDATE      (SCP_EVENT_BASE+3)
#define SCP_EVENT_SESSION_EVENT             (SCP_EVENT_BASE+4)
#define SCP_EVENT_SESSION_METRICS_UPDATE    (SCP_EVENT_BASE+5)

//! \class SCPEventNewSession
//!
//! \brief New session detected on the SCP.
//!
//! Details: A SCPSlave attachment to the SCP detects new sessions using
//!          DDS key life-cycle management. In case a new instance is detected
//!          a new session created by the SCPSlave object and posted to the
//!          upper layer protocol
//!
class SCPEventNewSession : public Event {
    
public:
    SCPEventNewSession(SCPObject *_session);
    SCPObject* GetSession();
    
private:
    SCPObject *session;
};

class SCPEventDeleteSession : public Event {
public:
    SCPEventDeleteSession(int _sessionId);
    int GetSessionId();
    ~SCPEventDeleteSession();
    
private:
    int sessionId;
};

//! \class SCPEventT
//!
//! \brief Template for Session Update, State and Metrics events
//!
//! Details: This template defines events for a session update (seen instance),
//!          state changes and metrics updates. Each event carries the IDL
//!          for the event
//!
template<class T,typename TypeSupport,int E>
class SCPEventT : public Event {
public:
    SCPEventT(T *_d) : Event(E)
    {
        d = TypeSupport::create_data();
        TypeSupport::copy_data(d,_d);
    }
    
    T* GetData()
    {
        return d;
    }
    
    ~SCPEventT()
    {
        TypeSupport::delete_data(d);
    }
    
private:
    T *d;
};

//! \class SCPEventUpdateSession
//!
//! \brief SCP session update event. This event is generated for not new 
//!        instances. The payload
typedef SCPEventT<com::xvd::neuron::session::Control,
                  com::xvd::neuron::session::ControlTypeSupport,
                  SCP_EVENT_UPDATE_SESSION> SCPEventUpdateSession;

//! \class SCPEventSessionStateUpdate
//!
//! \brief SCP session state event. This event is generated when a state
//!        udpate is detected
typedef SCPEventT<com::xvd::neuron::session::State,
                  com::xvd::neuron::session::StateTypeSupport,
                  SCP_EVENT_SESSION_STATE_UPDATE> SCPEventSessionStateUpdate;

//! \class SCPEventSessionEvent
//!
//! \brief SCP session event event. This event is generated when a new event
//!        on a sesssion is detected
typedef SCPEventT<com::xvd::neuron::session::Event,
                  com::xvd::neuron::session::EventTypeSupport,
                  SCP_EVENT_SESSION_EVENT> SCPEventSessionEvent;

//! \class SCPEventSessionMetricsUpdate
//!
//! \brief SCP session metrics event. This event is generated when a new metrics
//!        on a sesssion is detected
typedef SCPEventT<com::xvd::neuron::session::Metrics,
                  com::xvd::neuron::session::MetricsTypeSupport,
                  SCP_EVENT_SESSION_METRICS_UPDATE> SCPEventSessionMetricsUpdate;
#endif
