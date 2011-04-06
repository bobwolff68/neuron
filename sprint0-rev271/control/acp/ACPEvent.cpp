//!
//! \file ACPEvent.cpp
//!
//! \brief Implementation of ACP Events.
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#include "ACPEvent.h"
#include "ACPObject.h"

ACPEventNewSession::ACPEventNewSession(ACPObject *_session,
        com::xvd::neuron::acp::Control *_data) : Event(ACP_EVENT_NEW_SESSION)
{
    session = _session;
    data = com::xvd::neuron::acp::ControlTypeSupport::create_data();
    com::xvd::neuron::acp::ControlTypeSupport::copy_data(data,_data);
}


ACPObject* ACPEventNewSession::GetSession()
{
    return session;
}

com::xvd::neuron::acp::Control *ACPEventNewSession::GetData()
{
    return data;
}

ACPEventNewSession::~ACPEventNewSession()
{
    com::xvd::neuron::acp::ControlTypeSupport::delete_data(data);
}

ACPEventDeleteSession::ACPEventDeleteSession(int _sessionId) : Event(ACP_EVENT_DELETE_SESSION)
{
    sessionId = _sessionId;
}

int ACPEventDeleteSession::GetSessionId()
{
    return sessionId;
}

//! \brief State disposed event
//!
ACPEventSessionStateLost::ACPEventSessionStateLost(int _dstId) : Event(ACP_EVENT_SESSION_STATE_LOST)
{
    dstId = _dstId;
}

//! \brief Get instance if state disposed event
//!
int ACPEventSessionStateLost::GetDstId()
{
    return dstId;
}

