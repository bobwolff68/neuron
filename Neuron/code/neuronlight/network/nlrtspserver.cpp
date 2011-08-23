#include <stdio.h>
#include "nlrtspserver.h"

using namespace std;

nl_rtspserver_t* nl_rtspserver_t::createNew(
    UsageEnvironment& uenv,
    Port serv_port,
    UserAuthenticationDatabase* p_authdb,
    unsigned int reclamationTestSeconds
)
{
    int serv_sockd = setUpOurSocket(uenv,serv_port);
    if(serv_sockd==-1)  return NULL;
    return new nl_rtspserver_t(uenv,serv_sockd,serv_port,p_authdb,reclamationTestSeconds);
}

nl_rtspserver_t::nl_rtspserver_t(
    UsageEnvironment& uenv,
    int serv_sockd,
    Port serv_port,
    UserAuthenticationDatabase* p_authdb,
    unsigned int reclamationTestSeconds
):
RTSPServerSupportingHTTPStreaming(uenv,serv_sockd,serv_port,p_authdb,reclamationTestSeconds),
b_server_exit(0)
{
}

nl_rtspserver_t::~nl_rtspserver_t()
{
}

void nl_rtspserver_t::setup_sms(const char* stream_name)
{
    ServerMediaSession* p_sms = ServerMediaSession::createNew(envir(),stream_name,stream_name,NULL);
    OutPacketBuffer::maxSize = 100000;
    string video_stream_name = stream_name;
    string audio_stream_name = stream_name;
    
    video_stream_name += ".264";
    audio_stream_name += ".aac";
    
    p_sms->addSubsession(
        H264VideoFileServerMediaSubsession::createNew(
            envir(),
            video_stream_name.c_str(),
            True
        )
    );
    
    /*p_sms->addSubsession(
        ADTSAudioFileServerMediaSubsession::createNew(
            envir(),
            audio_stream_name.c_str(),
            True
        )
    );*/
    
    addServerMediaSession(p_sms);
}

int nl_rtspserver_t::workerBee(void)
{
    envir().taskScheduler().doEventLoop(&b_server_exit);
    return 0;
}