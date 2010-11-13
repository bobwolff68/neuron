#include "LSCPEvent.h"
#include "LSCPObject.h"

LSCPEventNewSession::LSCPEventNewSession(LSCPObject *_session,
        com::xvd::neuron::lscp::Control *_data) : Event(LSCP_EVENT_NEW_SESSION)
{
    session = _session;
    data = com::xvd::neuron::lscp::ControlTypeSupport::create_data();
    com::xvd::neuron::lscp::ControlTypeSupport::copy_data(data,_data);
}

LSCPObject* LSCPEventNewSession::GetSession()
{
    return session;
}

com::xvd::neuron::lscp::Control *LSCPEventNewSession::GetData()
{
    return data;
}

LSCPEventNewSession::~LSCPEventNewSession()
{
    com::xvd::neuron::lscp::ControlTypeSupport::delete_data(data);    
}

LSCPEventDeleteSession::LSCPEventDeleteSession(int _sessionId) : Event(LSCP_EVENT_DELETE_SESSION)
{
    sessionId = _sessionId;
}

int LSCPEventDeleteSession::GetSessionId()
{
    return sessionId;
}
