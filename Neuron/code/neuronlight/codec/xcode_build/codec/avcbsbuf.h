//
//  avcbsbuf.h
//  codec
//
//  Created by Manjesh Malavalli on 8/18/11.
//  Copyright 2011 XVDTH. All rights reserved.
//

#ifndef codec_avcbsbuf_h
#define codec_avcbsbuf_h

#include <stdlib.h>

class v4_avcbsbuf_t
{
private:
    unsigned char* p_membuf;
    int membuf_size;
    int stream_size;
    
public:
    v4_avcbsbuf_t():
    membuf_size(4),
    stream_size(4)
    {
        membuf_size = 4;
        p_membuf = (unsigned char*) malloc(membuf_size);
        p_membuf[0] = 0;
        p_membuf[1] = 0;
        p_membuf[2] = 0;
        p_membuf[3] = 1;
    }
    
    ~v4_avcbsbuf_t()
    {
        free(p_membuf);
    }
    
    void copy_nalu(const void* p_nalu_buf,const int nalu_size)
    {
        if(membuf_size < (nalu_size+4))
        {
            membuf_size = nalu_size + 4;
            p_membuf = (unsigned char*) realloc(p_membuf,membuf_size);
        }
        
        stream_size = nalu_size + 4;
        memcpy(p_membuf+4,p_nalu_buf,nalu_size);
    }
    
    unsigned char* get_membuf_ptr(void) const
    {
        return p_membuf;
    }
    
    int get_size(void)  const
    {
        return stream_size;
    }
};

#endif
