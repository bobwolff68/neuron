#include <iostream>
#include <map>
#include <string>
#include "nlrtcamstream.h"

using namespace std;


    // For testing only - ability to clear a/v queues in Live555 source and monitor their lengths
extern    bool bClearQueues;
extern    bool bQuit;
extern    int audioQueueLength;
extern    int videoQueueLength;
extern    int bitRate;
extern    bool bChangeBitrate;
extern    int frameRate;
extern    bool bChangeFramerate;

#if (defined(__APPLE__) & defined(__MACH__))                              
    nl_rtcamstream_t::nl_rtcamstream_t(TempVidCapBase* _p_cap,
                                       const short rtsp_port,
                                       const int width,
                                       const int height,
                                       const char* colorspace,
                                       const bool b_video_on,
                                       const bool b_audio_on):
    p_cap(_p_cap),p_serv(NULL)
#else
    nl_rtcamstream_t::nl_rtcamstream_t(const char* rtenc_cfg_file,
                                       const short rtsp_port,
                                       const char* width,
                                       const char* height,
                                       const char* colorspace)
#endif
{
    uint16_t backup_ports[6] = {0,554,50004,4100,5400,6001};
    
	try
	{
#if (!((defined(__APPLE__) & defined(__MACH__))))
		p_cap = new V4L2Cap("/dev/video0",i_width,i_height,"YUYV");
#endif
        backup_ports[0] = rtsp_port;
        p_bsdq = new SafeBufferDeque(1000);
        p_absdq = new SafeBufferDeque(5000);
		p_rtenc = new v4_rtenc_t(p_cap->GetBufferPointer(),p_absdq,b_video_on,b_audio_on);
        p_fifoout = new v4_fifoout_t(p_bsdq,p_rtenc);
        
        for(int i=0; i<5 && p_serv==NULL; i++)
        {
            cout << "Trying rtsp port: " << backup_ports[i] << endl;
            p_serv = nl_rtspserver_t::createNew(
                        *BasicUsageEnvironment::createNew(
                            *BasicTaskScheduler::createNew()
                        ),
                        backup_ports[i],
                        NULL,
                        65,
                        p_bsdq,
                        p_absdq
                     );
        }
        
        if(p_serv == NULL)
        {
            LOG_ERR("All attempts to instantiate rtsp server failed... aborting");
            throw RTCS_RETCODE_ERR_RTTRANS;
        }
        else
            cout << "Successful" << endl;
        
        //setup server media session "stream0"
        p_serv->setup_sms("stream0",b_video_on,b_audio_on);

		map<string,string> nvpairs;
        char s_width[20];
        char s_height[20];
        sprintf(s_width,"%d",width);
        sprintf(s_height,"%d",height);
		nvpairs["Width"] = s_width;
		nvpairs["Height"] = s_height;
		nvpairs["Colorspace"] = colorspace;
		p_rtenc->SetEncSettings(nvpairs);
	}
	catch(RTEnc_ReturnCode_t& rtenc_err_code)
	{
		LOG_ERR("real time encoder init error");
		throw RTCS_RETCODE_ERR_RTENC_FAIL;
	}
	catch(v4fifoout_retcode_t& fifoout_err_code)
	{
		LOG_ERR("Fifoout init error");
		throw RTCS_RETCODE_ERR_FIFOOUT_FAIL;
	}
    
    cout << "Before rtenc->Open()" << endl;
	if(p_rtenc->Open()!=RTENC_RETCODE_OK)
	{
		LOG_ERR("Real time encoder open error");
		throw RTCS_RETCODE_ERR_RTENC_OPEN;
	}
    
    if(p_rtenc->OpenAudio()!=RTENC_RETCODE_OK)
    {
        LOG_ERR("rtenc->OpenAudio() error");
        throw RTCS_RETCODE_ERR_RTENC_OPEN;
    }
    cout << "After rtenc->Open()" << endl;
}

nl_rtcamstream_t::~nl_rtcamstream_t()
{
	try
	{
        delete p_bsdq;
        delete p_absdq;
	    nl_rtspserver_t::destroy(p_serv);
		delete p_fifoout;
        p_rtenc->CloseAudio();
		delete p_rtenc;
#if( !(defined(__APPLE__) & defined(__MACH__)) )
		delete p_cap;
#else
        p_cap->release();
#endif
        cout << "Deleting rtcamstream instance..." << endl;
	}
	catch(RTEnc_ReturnCode_t& rtenc_err_code)
	{
		LOG_ERR("Real time encoder shutdown error");
		throw RTCS_RETCODE_ERR_RTENC_SHUTDOWN_FAIL;
	}
}

void nl_rtcamstream_t::RunCapture(void)
{
	try
	{
	    p_serv->startThread();
	    p_fifoout->startThread();
	    p_rtenc->startThread();
        
        // Sleep for about 2 seconds before testing for SDP presence.
        usleep(2000 * 1000);
        p_serv->test_sdp();
        
        while (!bQuit) {
            usleep(100 * 1000);
            videoQueueLength = p_bsdq->qsize();
            audioQueueLength = p_absdq->qsize();
            
            if (bClearQueues)
            {
                bClearQueues=false;
                p_bsdq->clearAll();
                p_absdq->clearAll();
            }
            if(bChangeBitrate)
            {
                bChangeBitrate = false;
                p_rtenc->LockHandle();
                p_rtenc->ChangeTargetBitrate(bitRate);
                p_rtenc->UnlockHandle();
            }
            if(bChangeFramerate)
            {
                bChangeFramerate = false;
                p_rtenc->LockHandle();
                p_rtenc->ChangeTargetFrameRate(frameRate);
                p_rtenc->UnlockHandle();
            }
        }
	
		//p_cap->stop_capturing();
		p_rtenc->stopThread();
		p_fifoout->stopThread();
        p_serv->request_server_exit();
        p_serv->stopThread();

	}
	catch(RTEnc_ReturnCode_t& rtenc_err_code)
	{
		LOG_ERR("Real time encoder error");
		throw RTCS_RETCODE_ERR_RTENC;
	}

	if(p_rtenc->Close()!=RTENC_RETCODE_OK)
	{
		LOG_ERR("Real time encoder close error");
		throw RTCS_RETCODE_ERR_RTENC_CLOSE;
	}
    
	return;
}