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

void nl_rtspserver_t::setup_sms(const char* stream_name,bool b_video_on,bool b_audio_on)
{
    ServerMediaSession* p_sms = ServerMediaSession::createNew(envir(),stream_name,stream_name,NULL);
    OutPacketBuffer::maxSize = 100000;
    string video_stream_name = stream_name;
    string audio_stream_name = stream_name;
    
    video_stream_name += VIDEO_FIFO_EXT;
    audio_stream_name += AUDIO_FIFO_EXT;
    
    if (b_video_on) 
    {
        p_sms->addSubsession(
                    H264VideoFileServerMediaSubsession::createNew(
                                        envir(),
                                        video_stream_name.c_str(),
                                        True
                    )
        );
    }
    
    if (b_audio_on) 
    {
        p_sms->addSubsession(
                //ADTSAudioFileServerMediaSubsession::createNew(
                MP3AudioFileServerMediaSubsession::createNew(
                                    envir(),
                                    audio_stream_name.c_str(),
                                    True,
                                    False,
                                    NULL
                )
        );
    }
    
    addServerMediaSession(p_sms);
}

int nl_rtspserver_t::workerBee(void)
{
    envir().taskScheduler().doEventLoop(&b_server_exit);
    return 0;
}