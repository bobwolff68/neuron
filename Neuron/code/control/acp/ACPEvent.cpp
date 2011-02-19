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


//! \brief Post delete session event
//!
//! \param [in] _sessionId    - session id to delete
//!
ACPEventSession::ACPEventSession(int _sessionId,int _event) : Event(_event)
{
    sessionId = _sessionId;
}

//! \brief Post delete session event
//!
//! \param [in] _sessionId    - session id to delete
//!
int ACPEventSession::GetSessionId()
{
	return sessionId;
}

//! \brief Post delete session event
//!
//! \param [in] _sessionId    - session id to delete
//!
ACPEventLostSession::ACPEventLostSession(int _sessionId) : ACPEventSession(_sessionId,ACP_EVENT_LOST_SESSION)
{
}

ACPEventDeleteSession::ACPEventDeleteSession(int _sessionId) : ACPEventSession(_sessionId,ACP_EVENT_DELETE_SESSION)
{
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

//! \brief State disposed event
//!
ACPEventSessionStateDisposed::ACPEventSessionStateDisposed(int _dstId) : Event(ACP_EVENT_SESSION_STATE_DISPOSED)
{
    dstId = _dstId;
}

//! \brief Get instance if state disposed event
//!
int ACPEventSessionStateDisposed::GetDstId()
{
    return dstId;
}
