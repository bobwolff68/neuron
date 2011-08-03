//
//  nlrtpstream.h
//  nlrtp
//
//  Created by Manjesh Malavalli on 8/2/11.
//  Copyright 2011 XVDTH. All rights reserved.
//

#ifndef NLRTPSTREAM_H_
#define NLRTPSTREAM_H_

#include <map>
#include <string>
#include "nlrtp.h"

typedef enum
{
    NL_RTPSTREAM_RETCODE_OK = 0,
    NL_RTPSTREAM_RETCODE_ERR_SOCK_CREAT,
    NL_RTPSTREAM_RETCODE_ERR_INV_IP,
    NL_RTPSTREAM_RETCODE_ERR_DEST_NOTFOUND
} nl_rtpstream_retcode_t;

class nl_rtpstream_t
{
protected:
    int32_t sockd;
    nl_rtp_packet_handler_t rtp_pkt_hdl;
    std::map<std::string,struct sockaddr_in> dests;
    
public:
    nl_rtpstream_t(const uint32_t ssrc);
    virtual ~nl_rtpstream_t();
    nl_rtpstream_retcode_t add_dest(const char* dest_ta);
    nl_rtpstream_retcode_t rem_dest(const char* dest_ta);
    nl_rtpstream_retcode_t dist_packet(
        const void* p_pay,
        const uint32_t pay_size,
        const uint32_t pay_type,
        const uint32_t marker,
        const uint32_t timestamp
    );
};

#endif
