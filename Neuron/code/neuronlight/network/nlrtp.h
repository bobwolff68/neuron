//
//  nlrtp.h
//  nlrtp
//
//  Created by Manjesh Malavalli on 8/2/11.
//  Copyright 2011 XVDTH. All rights reserved.
//

#ifndef NLRTP_H_
#define NLRTP_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#if (!defined(LITTLE_ENDIAN) && !defined(BIG_ENDIAN))
#define LITTLE_ENDIAN
#endif

#define RTP_VERSION         2
#define RTP_HDR_SIZE        12
#define RTP_MAX_PAY_SIZE    32768

#ifdef BUILD_DEBUG
    #define LOG_OUT(msg)    cout << __func__ << "> "\
                                 << msg << endl
#else
    #define LOG_OUT(msg)    ((void*)0)
#endif

#define LOG_ERR(msg)    cerr << __FILE__ << "|"\
                             << __LINE__ << "|"\
                             << __func__ << "> "\
                             << msg << endl

typedef enum
{
    NL_PKTHDL_RETCODE_OK = 0,
    NL_PKTHDL_RETCODE_ERR_SOCK_WRITE
} nl_pkthdl_retcode_t;

#ifdef LITTLE_ENDIAN
typedef struct
{
    unsigned int cc:4;  //rtp contributing source count
    unsigned int x:1;   //rtp header extension flag
    unsigned int p:1;   //rtp payload padding bit
    unsigned int v:2;   //rtp protocol version
    unsigned int pt:7;  //rtp payload type
    unsigned int m:1;   //rtp app-specific marker bit    
    uint16_t seqnum;    //sequence number
    uint32_t ts;        //timestamp
    uint32_t ssrc;      //synchronization source id
    
} nl_rtphdr_t;
#else
typedef struct
{
    unsigned int v:2;   //rtp protocol version
    unsigned int p:1;   //rtp payload padding bit
    unsigned int x:1;   //rtp header extension flag
    unsigned int cc:4;  //rtp contributing source count
    unsigned int m:1;   //rtp app-specific marker bit
    unsigned int pt:7;  //rtp payload type
    uint16_t seqnum;    //sequence number
    uint32_t ts;        //timestamp
    uint32_t ssrc;      //synchronization source id
    
} nl_rtphdr_t;
#endif

typedef struct
{
    nl_rtphdr_t header;
    uint8_t payload[RTP_MAX_PAY_SIZE];
} nl_rtppkt_t;

class nl_rtp_packet_handler_t
{
private:
    nl_rtppkt_t packet;
    
public:
    nl_rtp_packet_handler_t()
    {
        packet.header.v = 2;
        packet.header.p = 0;
        packet.header.x = 0;
        packet.header.cc = 0;
        packet.header.seqnum = (uint16_t) rand();
    }
    
    virtual ~nl_rtp_packet_handler_t()
    {
    }
    
    void set_m(uint32_t _m)
    {
        packet.header.m = _m;
    }
    
    void set_pt(uint32_t _pt)
    {
        packet.header.pt = _pt;
    }
    
    void set_seqnum(uint16_t _seqnum)
    {
        packet.header.seqnum = htons(_seqnum);
    }
    
    void set_ts(uint32_t _ts)
    {
        packet.header.ts = htonl(_ts);
    }
    
    void set_ssrc(uint32_t _ssrc)
    {
        packet.header.ssrc = htonl(_ssrc);
    }
    
    uint32_t get_pt(void) const
    {
        return packet.header.pt;
    }
    
    uint16_t get_seqnum(void) const
    {
        return (uint16_t) ntohs(packet.header.seqnum);
    }
    
    uint32_t get_ts(void) const
    {
        return (uint32_t) ntohl(packet.header.ts);
    }

    uint32_t get_ssrc(void) const
    {
        return ntohl(packet.header.ssrc);
    }

    nl_pkthdl_retcode_t write(
        int32_t sockd,
        struct sockaddr_in* p_dest,
        const void* p_pay,
        const uint32_t pay_size
    );
};

#endif
