//
//  cpp_main.h
//  rtcamstream
//
//  Created by Manjesh Malavalli on 8/9/11.
//  Copyright 2011 XVDTH. All rights reserved.
//

#ifndef rtcamstream_cpp_main_h
#define rtcamstream_cpp_main_h

#include <string>
#include <ThreadSingle.h>
#include "RTBuffer.h"

class RunPipeline : public ThreadSingle
{
private:
    TempVidCapBase* p_cap_objc;
    
    //declared void* because inclusion of rtcamstream.h will pollute
    //".mm" code with rtcamstream header code, which we did not want 
    void* pRTCamStream;
    
    const int width;
    const int height;
    const bool b_video_on;
    const bool b_audio_on;
    const string colorspace;
    virtual int workerBee(void);
    
public:
    RunPipeline(TempVidCapBase* _p_cap_objc,
                const int _width,
                const int _height,
                const char* _colorspace,
                const bool b_video_on,
                const bool b_audio_on);
    
    ~RunPipeline();
};

#endif