//
//  nlrtp.cpp
//  nlrtp
//
//  Created by Manjesh Malavalli on 8/2/11.
//  Copyright 2011 XVDTH. All rights reserved.
//

#include <iostream>
#include "nlrtp.h"

using namespace std;

nl_pkthdl_retcode_t nl_rtp_packet_handler_t::write(
    int32_t sockd,
    struct sockaddr_in* p_dest,
    const void* p_pay,
    const uint32_t pay_size
)
{
    const uint32_t pkt_size = RTP_HDR_SIZE+pay_size;
    
    memcpy((void*)packet.payload, p_pay, (size_t)pay_size);
    if (sendto(sockd,
               (const void*)&packet,
               (size_t)pkt_size,
               0,
               (const struct sockaddr*)p_dest,
               (socklen_t)sizeof(*p_dest)) != pkt_size)
    {
        LOG_ERR("sendto() error");
        return NL_PKTHDL_RETCODE_ERR_SOCK_WRITE;
    }
    
    packet.header.seqnum = htons(ntohs(packet.header.seqnum)+1);
    return NL_PKTHDL_RETCODE_OK;
}