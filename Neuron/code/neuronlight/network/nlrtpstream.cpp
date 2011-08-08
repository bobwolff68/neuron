//
//  nlrtpstream.cpp
//  nlrtp
//
//  Created by Manjesh Malavalli on 8/2/11.
//  Copyright 2011 XVDTH. All rights reserved.
//

#include <iostream>
#include "nlrtpstream.h"

using namespace std;

nl_rtpstream_t::nl_rtpstream_t(const uint32_t ssrc)
{
    LOG_OUT("Set ssrc=" << hex << ssrc << dec);
    
    //init packet header's ssrc field
    rtp_pkt_hdl.set_ssrc(ssrc);
    
    LOG_OUT("Create socket");
    
    //create socket
    sockd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sockd < 0)
    {
        LOG_ERR("socket() error");
        throw NL_RTPSTREAM_RETCODE_ERR_SOCK_CREAT;
    }
}

nl_rtpstream_t::~nl_rtpstream_t()
{
    dests.clear();
    close(sockd);
}

//dest_ta --> destination transport address (format: "port:ip")
nl_rtpstream_retcode_t nl_rtpstream_t::add_dest(const char* dest_ta)
{    
    struct sockaddr_in dest;
    uint16_t dest_port;
    char dest_ip[20];
    
    //set-up the sockaddr_in struct for the new destination
    sscanf(dest_ta,"%u:%s",&dest_port,dest_ip);
    
    LOG_OUT("Adding destination: " << dest_ip << ":" << dest_port);
    
    memset(&dest,0,sizeof(dest));
    dest.sin_family = PF_INET;
    dest.sin_port = htons(dest_port);
    if(inet_aton(dest_ip, &dest.sin_addr) == 0)
    {
        LOG_ERR("inet_aton() error");
        return NL_RTPSTREAM_RETCODE_ERR_INV_IP;
    }
    
    //add the structure to the map
    string dest_transport_addr = dest_ta;
    dests[dest_transport_addr] = dest;
    return NL_RTPSTREAM_RETCODE_OK;
}

nl_rtpstream_retcode_t nl_rtpstream_t::rem_dest(const char* dest_ta)
{
    string dest_transport_addr = dest_ta;
    map<string,struct sockaddr_in>::iterator it;
    
    if ((it=dests.find(dest_transport_addr)) == dests.end())
    {
        LOG_ERR("error: " << dest_ta << " not found");
        return NL_RTPSTREAM_RETCODE_ERR_DEST_NOTFOUND;
    }
    
    dests.erase(it);
    return NL_RTPSTREAM_RETCODE_OK;
}

nl_rtpstream_retcode_t nl_rtpstream_t::dist_packet(
    const void* p_pay,
    const uint32_t pay_size,
    const uint32_t pay_type,
    const uint32_t marker,
    const uint32_t timestamp
)
{
    map<string,struct sockaddr_in>::iterator it;
    
    rtp_pkt_hdl.set_m(marker);
    rtp_pkt_hdl.set_ts(timestamp);
    rtp_pkt_hdl.set_pt(pay_type);
    for(it=dests.begin(); it!=dests.end(); it++)
    {
        LOG_OUT("writing: to " << it->first);
        if(rtp_pkt_hdl.write(
                sockd,
                &it->second,
                p_pay,
                pay_size
           ) != NL_PKTHDL_RETCODE_OK)
            LOG_ERR("error: unable to write to " << it->first);
    }
    
    return NL_RTPSTREAM_RETCODE_OK;
}