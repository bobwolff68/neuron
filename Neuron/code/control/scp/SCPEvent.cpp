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
SCPEventSession::SCPEventSession(int _srcId,int _sessionId,int _event) : Event(_event)
{
    srcId = _srcId;
    sessionId = _sessionId;
}

//! \brief Post delete session event
//!
//! \param [in] _sessionId    - session id to delete
//!
int SCPEventSession::GetSessionId()
{
	return sessionId;
}

//! \brief Post delete session event
//!
//! \param [in] _sessionId    - session id to delete
//!
int SCPEventSession::GetSrcId()
{
	return srcId;
}

//! \brief Post delete session event
//!
//! \param [in] _sessionId    - session id to delete
//!
SCPEventLostSession::SCPEventLostSession(int _srcId,int _sessionId) : SCPEventSession(_srcId,_sessionId,SCP_EVENT_LOST_SESSION)
{
}

//! \brief Post delete session event
//!
//! \param [in] _sessionId    - session id to delete
//!
SCPEventDeleteSession::SCPEventDeleteSession(int _srcId,int _sessionId) : SCPEventSession(_srcId,_sessionId,SCP_EVENT_DELETE_SESSION)
{
}

//! \brief State disposed event
//!
SCPEventSessionStateLost::SCPEventSessionStateLost(int _dstId) : Event(SCP_EVENT_SESSION_STATE_LOST)
{
}

//! \brief State disposed event
//!
SCPEventSessionStateLost::SCPEventSessionStateLost(int _srcId,int _sessionId) : SCPEventSession(_srcId,_sessionId,SCP_EVENT_SESSION_STATE_LOST)
{
}
