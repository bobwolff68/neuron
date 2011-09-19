//
//  cpp_main.cpp
//  rtcamstream
//
//  Created by Manjesh Malavalli on 8/9/11.
//  Copyright 2011 XVDTH. All rights reserved.
//
#include "nlrtcamstream.h"
#include "cpp_main.h"

RunPipeline::RunPipeline(TempVidCapBase* _p_cap_objc,
                         const int _width,
                         const int _height,
                         const char* _colorspace,
                         const bool _b_video_on,
                         const bool _b_audio_on):
b_video_on(_b_video_on),
b_audio_on(_b_audio_on),
p_cap_objc(_p_cap_objc),
width(_width),
height(_height),
colorspace(_colorspace)
{
    pRTCamStream = new nl_rtcamstream_t(
                        p_cap_objc, 8554, 
                        width, height, 
                        colorspace.c_str(), 
                        b_video_on, b_audio_on
                   );
    startThread();
}

RunPipeline::~RunPipeline()
{
    stopThread();
    delete (nl_rtcamstream_t*)(pRTCamStream);
}

int RunPipeline::workerBee(void)
{
    ((nl_rtcamstream_t*)pRTCamStream)->RunCapture();
    return 0;
}

