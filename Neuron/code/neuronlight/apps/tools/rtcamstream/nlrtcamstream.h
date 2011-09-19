#ifndef NLRTCAMSTREAM_H_
#define NLRTCAMSTREAM_H_

#include "v4rtenc.h"
#include "v4fifoout.h"
#include "nlrtspserver.h"

//#define N_STREAMS   1

enum RTCS_ReturnCode_t
{
	RTCS_RETCODE_ERR_FIFOOUT_FAIL = -9,
	RTCS_RETCODE_ERR_LRS_FAIL,
	RTCS_RETCODE_ERR_RTENC_FAIL,
	RTCS_RETCODE_ERR_RTTRANS_SHUTDOWN_FAIL,
	RTCS_RETCODE_ERR_RTENC_SHUTDOWN_FAIL,
	RTCS_RETCODE_ERR_RTENC_OPEN,
	RTCS_RETCODE_ERR_RTENC_CLOSE,
	RTCS_RETCODE_ERR_RTTRANS,
	RTCS_RETCODE_ERR_RTENC,
	RTCS_RETCODE_OK
};

class nl_rtcamstream_t
{
private:
#if (defined(__APPLE__) & defined(__MACH__))
    TempVidCapBase* p_cap;
#else
	V4L2Cap* p_cap;
#endif
	v4_rtenc_t* p_rtenc;
    v4_fifoout_t* p_fifoout;
    SafeBufferDeque* p_absdq;
    SafeBufferDeque* p_bsdq;
	nl_rtspserver_t* p_serv;
    
public:
#if (defined(__APPLE__) & defined(__MACH__))
    static void main(TempVidCapBase* p_cap_objc,
                     const int width,
                     const int height,
                     const char* colorspace,
                     const bool b_video_on,
                     const bool b_audio_on);
    
	nl_rtcamstream_t(TempVidCapBase* _p_cap,
                     const short rtsp_port,
                     const int width,
                     const int height,
                     const char* colorspace,
                     const bool b_video_on,
                     const bool b_audio_on);
#else
	nl_rtcamstream_t(const char* rtenc_cfg_file,
                     const short rtsp_port,
                     const char* width,
                     const char* height,
                     const char* colorspace);
#endif
	~nl_rtcamstream_t();
	void RunCapture(void);
};

#endif
