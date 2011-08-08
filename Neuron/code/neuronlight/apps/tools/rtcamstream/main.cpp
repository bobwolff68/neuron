#include <iostream>
#include "nlrtcamstream.h"

#define STREAM_BUF_LEN 65536

using namespace std;

int main(int argc,char* argv[])
{
	if(argc!=6)
	{
		cout << "Usage: ./" << argv[0] << " <encoder-cfg-file> <rtsp_port> " 
             << "<width> <height> <colorspace>('YUYV' only for now)" << endl;
		return -1;
	}

	try
	{
	    int rtsp_port;
	    sscanf(argv[2],"%d",&rtsp_port);
		nl_rtcamstream_t cam_stream(argv[1],(const short)rtsp_port,argv[3],argv[4],argv[5]);
		cam_stream.RunCapture();
	}
	catch(RTCS_ReturnCode_t& rtcs_error_code)
	{
		LOG_ERR("real time cam stream error");
		return -1;
	}

	return 0;
}
