//!
//! \file SCPEvent.cpp
//!
//! \brief Implementation of SCP Events.
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#include "SCPEvent.h"
#include "SCPObject.h"

//! \brief Post new session event
//!
//! \param [in] _session - Session object
//! \param [in] _data    - Data for session
//!
SCPEventNewSession::SCPEventNewSession(SCPObject *_session,com::xvd::neuron::scp::Control *_data) : Event(SCP_EVENT_NEW_SESSION)
{
    session = _session;
    data = com::xvd::neuron::scp::ControlTypeSupport::create_data();
    com::xvd::neuron::scp::ControlTypeSupport::copy_data(data,_data);
}

//! \brief Get Session object from a session event
//!
//! \return session object from event
SCPObject* SCPEventNewSession::GetSession()
{
    return session;
}

//! \brief Get data from a session event
//!
//! \return session data
com::xvd::neuron::scp::Control *SCPEventNewSession::GetData()
{
    return data;
}

//! \brief Destructof for a session event
//!
//! \return session data
SCPEventNewSession::~SCPEventNewSession()
{
    com::xvd::neuron::scp::ControlTypeSupport::delete_data(data);    
}

//! \brief Post delete session event
//!
//! \param [in] _sessionId    - session id to delete
//!
SCPEventDeleteSession::SCPEventDeleteSession(int _sessionId) : Event(SCP_EVENT_DELETE_SESSION)
{
    sessionId = _sessionId;
}

//! \brief Get session id from session delete object
//!
//! \return session id in delete session object
//!
int SCPEventDeleteSession::GetSessionId()
{
    return sessionId;
}

//! \brief State disposed event
//!
SCPEventSessionStateDisposed::SCPEventSessionStateDisposed(int _dstId) : Event(SCP_EVENT_SESSION_STATE_DISPOSED)
{
    dstId = _dstId;
}

//! \brief Get instance if state disposed event
//!
int SCPEventSessionStateDisposed::GetDstId()
{
    return dstId;
}

//! \brief State disposed event
//!
SCPEventSessionStateLost::SCPEventSessionStateLost(int _dstId) : Event(SCP_EVENT_SESSION_STATE_UPDATE)
{
    dstId = _dstId;
}

//! \brief Get instance if state disposed event
//!
int SCPEventSessionStateLost::GetDstId()
{
    return dstId;
}
