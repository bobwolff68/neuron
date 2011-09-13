#include <stdio.h>
#include "nlrtspserver.h"

using namespace std;

nl_rtspserver_t* nl_rtspserver_t::createNew(
    UsageEnvironment& uenv,
    Port serv_port,
    UserAuthenticationDatabase* p_authdb,
    unsigned int reclamationTestSeconds,
    SafeBufferDeque* _p_bsdq,
    SafeBufferDeque* _p_absdq
)
{
    int serv_sockd = setUpOurSocket(uenv,serv_port);
    if(serv_sockd==-1)  return NULL;
    return new nl_rtspserver_t(uenv,serv_sockd,serv_port,p_authdb,reclamationTestSeconds,_p_bsdq,_p_absdq);
}

nl_rtspserver_t::nl_rtspserver_t(
    UsageEnvironment& uenv,
    int serv_sockd,
    Port serv_port,
    UserAuthenticationDatabase* p_authdb,
    unsigned int reclamationTestSeconds,
    SafeBufferDeque* _p_bsdq,
    SafeBufferDeque* _p_absdq
):
p_bsdq(_p_bsdq),
p_absdq(_p_absdq),
RTSPServerSupportingHTTPStreaming(uenv,serv_sockd,serv_port,p_authdb,reclamationTestSeconds),
b_server_exit(0)
{
}

nl_rtspserver_t::~nl_rtspserver_t()
{
    cout << "Deleting rtspserver instance..." << endl;
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
        p_sms->addSubsession(H264VideoDeviceServerMediaSubsession::createNew(envir(), True, p_bsdq));
    }
    
    if (b_audio_on) 
    {
        p_sms->addSubsession(ADTSAudioDeviceServerMediaSubsession::createNew(envir(), True, p_absdq));
    }
    
    addServerMediaSession(p_sms);
}

void nl_rtspserver_t::test_sdp(void)
{
    ServerMediaSession* p_sms = NULL;
    RTSPClient* p_testclient = NULL;
    char* sms_url = NULL;
    
    p_sms = lookupServerMediaSession("stream0");
    if(p_sms==NULL)
    {
        cerr << "Warning... unable to get media session for 'stream0'..." << endl;
        return;
    }
    
    sms_url = rtspURL(p_sms);
    if(sms_url == NULL)
    {
        cerr << "Warning... unable to get url for media session 'stream0'..." << endl;
        return;
    }
    else
    {
        cout << "URL: " << sms_url << endl;
    }
    
    p_testclient = RTSPClient::createNew(envir(), sms_url);
    if(p_testclient == NULL)
    {
        cerr << "Warning... unable to create rtsp client... test incomplete..." << endl;
        return;
    }
    
    p_testclient->sendDescribeCommand(after_describe_request);
    
    //TODO: find a way to delete p_testclient
    
    delete []sms_url;
}

void nl_rtspserver_t::after_describe_request(RTSPClient* p_client,int result_code,char* result_string)
{
    if (result_code != 0) {
        cerr << "Unable to obtain SDP... " << result_string << endl;
    }
    else {
        cout << "SDP creation successful..." << endl;
    }
}

int nl_rtspserver_t::workerBee(void)
{
    envir().taskScheduler().doEventLoop(&b_server_exit);
    return 0;
}