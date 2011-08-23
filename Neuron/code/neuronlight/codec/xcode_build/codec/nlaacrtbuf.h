//
//  nlaacrtbuf.h
//  codec
//
//  Created by Manjesh Malavalli on 8/22/11.
//  Copyright 2011 XVDTH. All rights reserved.
//

#ifndef codec_nlaacrtbuf_h
#define codec_nlaacrtbuf_h

#include <stdlib.h>
#include <RTBuffer.h>

class nl_aacrtbuf_t : public RTBuffer
{
public:
    nl_aacrtbuf_t()
    {
    }
    
    virtual ~nl_aacrtbuf_t()
    {
    }
    
    virtual bool EmptyBufferRelease(RTBufferInfoBase* pBI)
    {
        if(pBI->pBuffer)
            free(pBI->pBuffer);
        
        delete pBI;
        return true;
    }
};

class nl_aacbufinfo_t : public RTBufferInfoBase
{    
public:
    int bytes;
    nl_aacbufinfo_t()
    {
    }
    
    virtual ~nl_aacbufinfo_t()
    {
    }
};

#endif
