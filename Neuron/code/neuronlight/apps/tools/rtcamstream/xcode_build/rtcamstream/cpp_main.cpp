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
                         const char* _colorspace):
p_cap_objc(_p_cap_objc),
width(_width),
height(_height),
colorspace(_colorspace)
{
    startThread();
}

int RunPipeline::workerBee(void)
{
    nl_rtcamstream_t::main(p_cap_objc, width, height, colorspace.c_str());
    return 0;
}

