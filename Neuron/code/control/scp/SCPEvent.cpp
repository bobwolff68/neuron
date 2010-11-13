#include "SCPEvent.h"
#include "SCPObject.h"

SCPEventNewSession::SCPEventNewSession(SCPObject *_session,com::xvd::neuron::scp::Control *_data) : Event(SCP_EVENT_NEW_SESSION)
{
    session = _session;
    data = com::xvd::neuron::scp::ControlTypeSupport::create_data();
    com::xvd::neuron::scp::ControlTypeSupport::copy_data(data,_data);
}

SCPObject* SCPEventNewSession::GetSession()
{
    return session;
}

com::xvd::neuron::scp::Control *SCPEventNewSession::GetData()
{
    return data;
}

SCPEventNewSession::~SCPEventNewSession()
{
    com::xvd::neuron::scp::ControlTypeSupport::delete_data(data);    
}

SCPEventDeleteSession::SCPEventDeleteSession(int _sessionId) : Event(SCP_EVENT_DELETE_SESSION)
{
    sessionId = _sessionId;
}

int SCPEventDeleteSession::GetSessionId()
{
    return sessionId;
}
