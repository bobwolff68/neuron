#ifndef NLRTCAMSTREAM_H_
#define NLRTCAMSTREAM_H_

#include "V4L2Cap.h"
#include "v4rtenc.h"
#include "v4fifoout.h"
#include "nlrtspserver.h"

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
	V4L2Cap* p_cap;
	v4_rtenc_t* p_rtenc;
    v4_fifoout_t* p_fifoout;
	nl_rtspserver_t* p_serv;
	virtual void IdleLoop(void);

public:
	nl_rtcamstream_t(const char* rtenc_cfg_file,
                     const short rtsp_port,
                     const char* width,
                     const char* height,
                     const char* colorspace);
	~nl_rtcamstream_t();
	void RunCapture(void);
};

#endif
