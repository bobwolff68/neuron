#ifndef NLRTSPSERVER_H_
#define NLRTSPSERVER_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <liveMedia.hh>
#include <ThreadSingle.h>
#include <BasicUsageEnvironment.hh>
//#ifndef _RTSP_SERVER_SUPPORTING_HTTP_STREAMING_HH
//    #include "RTSPServerSupportingHTTPStreaming.hh"
//#endif

class nl_rtspserver_t : public RTSPServerSupportingHTTPStreaming, public ThreadSingle 
{
private:
    char b_server_exit;
    
    nl_rtspserver_t(
        UsageEnvironment& uenv,
        int serv_sockd,
        Port serv_port,
        UserAuthenticationDatabase* p_authdb,
        unsigned reclamationTestSeconds
    );
    virtual ~nl_rtspserver_t();
    virtual int workerBee(void);
    
public:
    static nl_rtspserver_t* createNew(
        UsageEnvironment& uenv,
        Port serv_port,
        UserAuthenticationDatabase* p_authdb=NULL,
        unsigned reclamationTestSeconds=65
    );

    static void destroy(nl_rtspserver_t* p_serv_instance)
    {
        delete p_serv_instance;
    }
    
    void setup_sms(const char* stream_name);
    
    void request_server_exit(void)
    {
        b_server_exit = 1;
    }
};

#endif