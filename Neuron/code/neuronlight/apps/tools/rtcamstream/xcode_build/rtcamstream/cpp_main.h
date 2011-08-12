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
    const int width;
    const int height;
    const string colorspace;
    virtual int workerBee(void);
    
public:
    RunPipeline(TempVidCapBase* _p_cap_objc,
                const int _width,
                const int _height,
                const char* _colorspace);
    
    ~RunPipeline()
    {
        stopThread();
    }
};

#endif