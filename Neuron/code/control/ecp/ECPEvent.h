//!
//! \file ECPEvent.h
//!
//! \brief Defintion of Session Control Plane (ECP) events
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#ifndef ECP_EVENT_H_
#define ECP_EVENT_H_

#include "neuroncommon.h"
#include "ECPInterface.h"
#include "ECPInterfaceSupport.h"
#include "ECPObject.h"

#define ECP_EVENT_BASE                      0x00000200
#define ECP_EVENT_NEW_SESSION               (ECP_EVENT_BASE+0)
#define ECP_EVENT_UPDATE_SESSION            (ECP_EVENT_BASE+1)
#define ECP_EVENT_DELETE_SESSION            (ECP_EVENT_BASE+2)
#define ECP_EVENT_SESSION_STATE_UPDATE      (ECP_EVENT_BASE+3)
#define ECP_EVENT_SESSION_EVENT             (ECP_EVENT_BASE+4)
#define ECP_EVENT_SESSION_METRICS_UPDATE    (ECP_EVENT_BASE+5)

//! \class ECPEventNewSession
//!
//! \brief New session detected on the ECP.
//!
//! Details: A ECPSlave attachment to the ECP detects new sessions using
//!          DDS key life-cycle management. In case a new instance is detected
//!          a new session created by the ECPSlave object and posted to the
//!          upper layer protocol
//!
class ECPEventNewSession : public Event {
    
public:
    ECPEventNewSession(ECPObject *_session,com::xvd::neuron::ecp::Control *_data);
    ECPObject* GetSession();
    com::xvd::neuron::ecp::Control *GetData();
    ~ECPEventNewSession();
private:
    ECPObject *session;
    com::xvd::neuron::ecp::Control *data;
};

class ECPEventDeleteSession : public Event {
public:
    ECPEventDeleteSession(int _sessionId);
    int GetSessionId();
    ~ECPEventDeleteSession();
    
private:
    int sessionId;
};

//! \class ECPEventT
//!
//! \brief Template for Session Update, State and Metrics events
//!
//! Details: This template defines events for a session update (seen instance),
//!          state changes and metrics updates. Each event carries the IDL
//!          for the event
//!
template<class T,typename TypeSupport,int E>
class ECPEventT : public Event {
public:
    ECPEventT(T *_d,DDS_SampleInfo *_i) : Event(E)
    {
        d = TypeSupport::create_data();
        TypeSupport::copy_data(d,_d);
    }
    
    T* GetData()
    {
        return d;
    }
    
    DDS_SampleInfo* GetSampleInfo()
    {
        return &info;
    }
    
    ~ECPEventT()
    {
        TypeSupport::delete_data(d);
    }
    
private:
    T *d;
    DDS_SampleInfo info;
};

//! \class ECPEventUpdateSession
//!
//! \brief ECP session update event. This event is generated for not new 
//!        instances. The payload
typedef ECPEventT<com::xvd::neuron::ecp::Control,
                  com::xvd::neuron::ecp::ControlTypeSupport,
                  ECP_EVENT_UPDATE_SESSION> ECPEventUpdateSession;

//! \class ECPEventSessionStateUpdate
//!
//! \brief ECP session state event. This event is generated when a state
//!        udpate is detected
typedef ECPEventT<com::xvd::neuron::ecp::State,
                  com::xvd::neuron::ecp::StateTypeSupport,
                  ECP_EVENT_SESSION_STATE_UPDATE> ECPEventSessionStateUpdate;

//! \class ECPEventSessionEvent
//!
//! \brief ECP session event event. This event is generated when a new event
//!        on a sesssion is detected
typedef ECPEventT<com::xvd::neuron::ecp::Event,
                  com::xvd::neuron::ecp::EventTypeSupport,
                  ECP_EVENT_SESSION_EVENT> ECPEventSessionEvent;

//! \class ECPEventSessionMetricsUpdate
//!
//! \brief ECP session metrics event. This event is generated when a new metrics
//!        on a sesssion is detected
typedef ECPEventT<com::xvd::neuron::ecp::Metrics,
                  com::xvd::neuron::ecp::MetricsTypeSupport,
                  ECP_EVENT_SESSION_METRICS_UPDATE> ECPEventSessionMetricsUpdate;
#endif
