//
//  nlfifostream.h
//  codec
//
//  Created by Manjesh Malavalli on 8/18/11.
//  Copyright 2011 XVDTH. All rights reserved.
//

#ifndef codec_nlfifostream_h
#define codec_nlfifostream_h

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <string>
#include <v4_nalu.h>
#include "avcbsbuf.h"

#define VIDEO_FIFO_EXT  ".264"
#define AUDIO_FIFO_EXT  ".mp3"

typedef enum
{
    NLFS_RETCODE_OK = 0,
    NLFS_RETCODE_ERR_EXCEPT,
    NLFS_RETCODE_ERR_CLOSE,
    NLFS_RETCODE_ERR_WRITE
} nlfs_retcode_t;

class nl_fifostream_t
{
private:
    int sfd[2];                     //video and audio fifo descriptors
    const std::string stream_name;  //common stream name for audio & video streams
    
    //Video buffers with startcode prefix
    v4_avcbsbuf_t* p_avcbs_buf;
    v4_avcbsbuf_t* p_sps_buf;
    v4_avcbsbuf_t* p_pps_buf;
    bool b_sps_sent;
    bool b_pps_sent;
    
    nlfs_retcode_t write_to_substr_fifo(
        const uint8_t* p_ssbuf,
        const int ssbuf_bytes,
        const bool b_is_video
    );
    
public:
    nl_fifostream_t(const char* _stream_name);
    virtual ~nl_fifostream_t();    
    nlfs_retcode_t write_to_substream(
        const uint8_t* p_outbuf,
        const int outbuf_bytes,
        const bool b_is_video
    );
};

#endif
