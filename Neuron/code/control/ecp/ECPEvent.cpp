//!
//! \file ECPEvent.cpp
//!
//! \brief Defintion of SCP Master Object
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#include "ECPEvent.h"
#include "ECPObject.h"

ECPEventNewSession::ECPEventNewSession(ECPObject *_session,
                com::xvd::neuron::ecp::Control *_data) : Event(ECP_EVENT_NEW_SESSION)
{
    session = _session;
    data = com::xvd::neuron::ecp::ControlTypeSupport::create_data();
    com::xvd::neuron::ecp::ControlTypeSupport::copy_data(data,_data);
}

ECPObject* ECPEventNewSession::GetSession()
{
    return session;
}

com::xvd::neuron::ecp::Control *ECPEventNewSession::GetData()
{
    return data;
}

ECPEventNewSession::~ECPEventNewSession()
{
    com::xvd::neuron::ecp::ControlTypeSupport::delete_data(data);    
}

ECPEventDeleteSession::ECPEventDeleteSession(int _sessionId) : Event(ECP_EVENT_DELETE_SESSION)
{
    sessionId = _sessionId;
}

int ECPEventDeleteSession::GetSessionId()
{
    return sessionId;
}
