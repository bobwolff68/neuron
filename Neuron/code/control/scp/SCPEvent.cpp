#include "SCPEvent.h"
#include "SCPObject.h"

SCPEventNewSession::SCPEventNewSession(SCPObject *_session) : Event(SCP_EVENT_NEW_SESSION)
{
    session = _session;
}

SCPObject* SCPEventNewSession::GetSession()
{
    return session;
}

SCPEventDeleteSession::SCPEventDeleteSession(int _sessionId) : Event(SCP_EVENT_DELETE_SESSION)
{
    sessionId = _sessionId;
}

int SCPEventDeleteSession::GetSessionId()
{
    return sessionId;
}
