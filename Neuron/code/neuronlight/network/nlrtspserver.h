#ifndef NLRTSPSERVER_H_
#define NLRTSPSERVER_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <liveMedia.hh>
#include <ThreadSingle.h>
#include <BasicUsageEnvironment.hh>
#include "H264VideoDeviceServerMediaSubsession.h"
#include "ADTSAudioDeviceServerMediaSubsession.h"

#define VIDEO_FIFO_EXT  ".264"
#define AUDIO_FIFO_EXT  ".mp3"

class nl_rtspserver_t : public RTSPServerSupportingHTTPStreaming, public ThreadSingle 
{
private:
    char b_server_exit;
    SafeBufferDeque* p_bsdq;
    SafeBufferDeque* p_absdq;
    
    nl_rtspserver_t(
        UsageEnvironment& uenv,
        int serv_sockd,
        Port serv_port,
        UserAuthenticationDatabase* p_authdb,
        unsigned reclamationTestSeconds,
        SafeBufferDeque* _p_bsdq,
        SafeBufferDeque* _p_absdq
    );
    virtual ~nl_rtspserver_t();
    virtual int workerBee(void);
    static void after_describe_request(RTSPClient* p_client,int result_code,char* result_string);
    
public:
    static nl_rtspserver_t* createNew(
        UsageEnvironment& uenv,
        Port serv_port,
        UserAuthenticationDatabase* p_authdb=NULL,
        unsigned reclamationTestSeconds=65,
        SafeBufferDeque* _p_bsdq = NULL,
        SafeBufferDeque* _p_absdq = NULL
    );

    static void destroy(nl_rtspserver_t* p_serv_instance)
    {
        delete p_serv_instance;
    }
    
    void setup_sms(const char* stream_name,bool b_video_on=true,bool b_audio_on=true);
    void test_sdp(void);
    
    void request_server_exit(void)
    {
        b_server_exit = 1;
    }
    
    UsageEnvironment& get_usage_environment(void) const
    {
        return envir();
    }
};

#endif