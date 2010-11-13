//!
//! \file LSCPEvent.h
//!
//! \brief Defintion of Session Control Plane (LSCP) events
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#ifndef LSCP_EVENT_H_
#define LSCP_EVENT_H_

#include "neuroncommon.h"
#include "LSCPInterface.h"
#include "LSCPInterfaceSupport.h"
#include "LSCPObject.h"

#define LSCP_EVENT_BASE                      0x00000300
#define LSCP_EVENT_NEW_SESSION               (LSCP_EVENT_BASE+0)
#define LSCP_EVENT_UPDATE_SESSION            (LSCP_EVENT_BASE+1)
#define LSCP_EVENT_DELETE_SESSION            (LSCP_EVENT_BASE+2)
#define LSCP_EVENT_SESSION_STATE_UPDATE      (LSCP_EVENT_BASE+3)
#define LSCP_EVENT_SESSION_EVENT             (LSCP_EVENT_BASE+4)
#define LSCP_EVENT_SESSION_METRICS_UPDATE    (LSCP_EVENT_BASE+5)

//! \class LSCPEventNewSession
//!
//! \brief New session detected on the LSCP.
//!
//! Details: A LSCPSlave attachment to the LSCP detects new sessions using
//!          DDS key life-cycle management. In case a new instance is detected
//!          a new session created by the LSCPSlave object and posted to the
//!          upper layer protocol
//!
class LSCPEventNewSession : public Event {
    
public:
    LSCPEventNewSession(LSCPObject *_session,com::xvd::neuron::lscp::Control *_data);
    LSCPObject* GetSession();
    com::xvd::neuron::lscp::Control *GetData();
    ~LSCPEventNewSession();

private:
    LSCPObject *session;
    com::xvd::neuron::lscp::Control *data;    
};

class LSCPEventDeleteSession : public Event {
public:
    LSCPEventDeleteSession(int _sessionId);
    int GetSessionId();
    ~LSCPEventDeleteSession();

private:
    int sessionId;
};

//! \class LSCPEventT
//!
//! \brief Template for Session Update, State and Metrics events
//!
//! Details: This template defines events for a session update (seen instance),
//!          state changes and metrics updates. Each event carries the IDL
//!          for the event
//!
template<class T,typename TypeSupport,int E>
class LSCPEventT : public Event {
public:
    LSCPEventT(T *_d) : Event(E)
    {
        d = TypeSupport::create_data();
        TypeSupport::copy_data(d,_d);
    }
    
    T* GetData()
    {
        return d;
    }
    
    ~LSCPEventT()
    {
        TypeSupport::delete_data(d);
    }
    
private:
    T *d;
};

//! \class LSCPEventUpdateSession
//!
//! \brief LSCP session update event. This event is generated for not new 
//!        instances. The payload
typedef LSCPEventT<com::xvd::neuron::lscp::Control,
                  com::xvd::neuron::lscp::ControlTypeSupport,
                  LSCP_EVENT_UPDATE_SESSION> LSCPEventUpdateSession;

//! \class LSCPEventSessionStateUpdate
//!
//! \brief LSCP session state event. This event is generated when a state
//!        udpate is detected
typedef LSCPEventT<com::xvd::neuron::lscp::State,
                  com::xvd::neuron::lscp::StateTypeSupport,
                  LSCP_EVENT_SESSION_STATE_UPDATE> LSCPEventSessionStateUpdate;

//! \class LSCPEventSessionEvent
//!
//! \brief LSCP session event event. This event is generated when a new event
//!        on a sesssion is detected
typedef LSCPEventT<com::xvd::neuron::lscp::Event,
                  com::xvd::neuron::lscp::EventTypeSupport,
                  LSCP_EVENT_SESSION_EVENT> LSCPEventSessionEvent;

//! \class LSCPEventSessionMetricsUpdate
//!
//! \brief LSCP session metrics event. This event is generated when a new metrics
//!        on a sesssion is detected
typedef LSCPEventT<com::xvd::neuron::lscp::Metrics,
                  com::xvd::neuron::lscp::MetricsTypeSupport,
                  LSCP_EVENT_SESSION_METRICS_UPDATE> LSCPEventSessionMetricsUpdate;
#endif
