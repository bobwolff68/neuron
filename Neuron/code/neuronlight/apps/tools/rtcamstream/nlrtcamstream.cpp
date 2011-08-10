#include <iostream>
#include <map>
#include <string>
#include "nlrtcamstream.h"

using namespace std;

#if (defined(__APPLE__) & defined(__MACH__))
    void nl_rtcamstream_t::main(TempVidCapBase* p_cap_objc,
                                const int width,
                                const int height,
                                const char* colorspace)
    {
        nl_rtcamstream_t rtcs(p_cap_objc,"rtenc_avc_settings.cfg",8554,width,height,colorspace);
        rtcs.RunCapture();
        //p_cap_objc->Release();
        return;
    }
                              
    nl_rtcamstream_t::nl_rtcamstream_t(TempVidCapBase* _p_cap,
                                       const char* rtenc_cfg_file,
                                       const short rtsp_port,
                                       const int width,
                                       const int height,
                                       const char* colorspace):
    p_cap(_p_cap)
#else
    nl_rtcamstream_t::nl_rtcamstream_t(const char* rtenc_cfg_file,
                                       const short rtsp_port,
                                       const char* width,
                                       const char* height,
                                       const char* colorspace)
#endif
{
	try
	{
#if (!((defined(__APPLE__) & defined(__MACH__))))
		p_cap = new V4L2Cap("/dev/video0",i_width,i_height,"YUYV");
#endif
		p_rtenc = new v4_rtenc_t(rtenc_cfg_file,p_cap->GetBufferPointer());
        p_fifoout = new v4_fifoout_t("stream.264",p_rtenc);
		/*p_serv = nl_rtspserver_t::createNew(
                        *BasicUsageEnvironment::createNew(
                            *BasicTaskScheduler::createNew()
                        ),
                        rtsp_port
                     );
        p_serv->setup_sms("stream.264");*/
        
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

	if(p_rtenc->Open()!=RTENC_RETCODE_OK)
	{
		LOG_ERR("Real time encoder open error");
		throw RTCS_RETCODE_ERR_RTENC_OPEN;
	}
}

nl_rtcamstream_t::~nl_rtcamstream_t()
{
	try
	{
	    //nl_rtspserver_t::destroy(p_serv);
		delete p_fifoout;
		delete p_rtenc;
#if( !(defined(__APPLE__) & defined(__MACH__)) )
		delete p_cap;
#endif
	}
	catch(RTEnc_ReturnCode_t& rtenc_err_code)
	{
		LOG_ERR("Real time encoder shutdown error");
		throw RTCS_RETCODE_ERR_RTENC_SHUTDOWN_FAIL;
	}
}

void nl_rtcamstream_t::IdleLoop(void)
{
	char ch;

	cerr << "Press any character followed by the return key to stop capturing..." << endl;
	cin >> ch;
	cerr << "Stopping capture..." << endl;

	return;
}

void nl_rtcamstream_t::RunCapture(void)
{
	try
	{
	    //p_serv->startThread();
	    p_fifoout->startThread();
	    p_rtenc->startThread();
		//p_cap->start_capturing();

		IdleLoop();
	
		//p_cap->stop_capturing();
		p_rtenc->stopThread();
		p_fifoout->stopThread();
        //p_serv->stopThread();
        //p_serv->request_server_exit();

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