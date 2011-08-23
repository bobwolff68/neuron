//
//  nlfifostream.cpp
//  codec
//
//  Created by Manjesh Malavalli on 8/18/11.
//  Copyright 2011 XVDTH. All rights reserved.
//

#include <iostream>
#include "nlfifostream.h"

using namespace std;

struct sigaction ign_sigpipe_act;

nl_fifostream_t::nl_fifostream_t(const char* _stream_name):
b_sps_sent(false),
b_pps_sent(false),
stream_name(_stream_name)
{
    string substream_names[2];

    //Ignore SIGPIPE signal
    ign_sigpipe_act.sa_handler = SIG_IGN;
    sigemptyset(&ign_sigpipe_act.sa_mask);
    ign_sigpipe_act.sa_flags = 0;
    sigaction(SIGPIPE, &ign_sigpipe_act, NULL);
    
    sfd[0] = sfd[1] = -1;
    substream_names[0] = stream_name + ".264";
    substream_names[1] = stream_name + ".aac";

    p_avcbs_buf = new v4_avcbsbuf_t();
    p_sps_buf = new v4_avcbsbuf_t();
    p_pps_buf = new v4_avcbsbuf_t();

    //If fifos don't exist for any substream, create them
    for (int i=0; i<2; i++) 
    {
        if (access(substream_names[i].c_str(),F_OK) == -1) 
        {
            if (mkfifo(substream_names[i].c_str(),S_IRUSR|S_IWUSR) == -1)
            {
                cerr << __func__ 
                     << "() error: mkfifo(" 
                     << substream_names[i] 
                     << ")..." << endl;
                throw NLFS_RETCODE_ERR_EXCEPT;
            }
        }
    }
}

nl_fifostream_t::~nl_fifostream_t()
{
    for (int i=0; i<2; i++)
    {
        if (sfd[i] != -1) 
        {
            if (close(sfd[i]) < 0) 
            {
                cerr << "Close error(" << i << ")" << endl;
                throw NLFS_RETCODE_ERR_CLOSE;
            }
        }
    }
    
    delete p_avcbs_buf;
    delete p_sps_buf;
    delete p_pps_buf;
}

nlfs_retcode_t nl_fifostream_t::write_to_substr_fifo(
    const uint8_t* p_ssbuf,
    const int ssbuf_bytes,
    const bool b_is_video
)
{
    /*fd_set sfd_set;
    struct timeval timeout;*/
    int sfd_idx = b_is_video ? 0 : 1;
    int ssbuf_total_bytes_written = 0;
    int ssbuf_bytes_written = 0;
        
    while (1) 
    {
        ssbuf_bytes_written = write(sfd[sfd_idx], p_ssbuf+ssbuf_total_bytes_written, (ssbuf_bytes-ssbuf_total_bytes_written));
        
        if(ssbuf_bytes_written < 0)
        {
            if(errno == EPIPE || errno != EAGAIN)
            {
                cout << "EPIPE = " << strerror(errno)  << " " << b_is_video << endl;
                close(sfd[sfd_idx]);
                
                sfd[sfd_idx] = -1;
                if(b_is_video)
                {
                    b_sps_sent = false;
                    b_pps_sent = false;
                }
                return NLFS_RETCODE_OK;
            }
        }
        else
        {
            ssbuf_total_bytes_written += ssbuf_bytes_written;
            if(ssbuf_total_bytes_written == ssbuf_bytes)
                break;
        }
    }
    
    return NLFS_RETCODE_OK;
}

nlfs_retcode_t nl_fifostream_t::write_to_substream(
    const uint8_t* p_outbuf,
    const int outbuf_bytes,
    const bool b_is_video
)
{
    if (b_is_video) 
    {
        int nalu_type = NALU_TYPE(p_outbuf[0]);
        int nalu_ridc = NALU_RIDC(p_outbuf[0]);

        /*cout << __func__ << "> nalu_type="
                << nalu_type
                << ", nalu_ridc="
                << nalu_ridc
                << ", size="
                << outbuf_bytes
                << " (bytes)" << endl;*/

        
        if (sfd[0] == -1) 
            sfd[0] = open((stream_name+".264").c_str(),O_WRONLY|O_NONBLOCK);
        
        if (nalu_type == NALU_TYPE_SPS) 
        {
            if(!b_sps_sent)
                p_sps_buf->copy_nalu(p_outbuf, outbuf_bytes);
        }
        else if (nalu_type == NALU_TYPE_PPS)
        {
            if(!b_pps_sent)
                p_pps_buf->copy_nalu(p_outbuf, outbuf_bytes);
        }
        else
        {        
            if (sfd[0] != -1) 
            {
                if(nalu_type == NALU_TYPE_IDR || nalu_ridc > 1)
                {
                    if(!b_sps_sent)
                    {
                        if(write_to_substr_fifo(p_sps_buf->get_membuf_ptr(),
                                                p_sps_buf->get_size(), 
                                                b_is_video) == NLFS_RETCODE_OK)
                            b_sps_sent = true;
                        else
                            return NLFS_RETCODE_ERR_WRITE;
                    }
                    if(!b_pps_sent)
                    {
                        if(write_to_substr_fifo(p_pps_buf->get_membuf_ptr(),
                                                p_pps_buf->get_size(), 
                                                b_is_video) == NLFS_RETCODE_OK)
                            b_pps_sent = true;
                        else
                            return NLFS_RETCODE_ERR_WRITE;
                    }
                }
                
                if(b_sps_sent && b_pps_sent)
                {
                    p_avcbs_buf->copy_nalu(p_outbuf, outbuf_bytes);
                    if(write_to_substr_fifo(p_avcbs_buf->get_membuf_ptr(), 
                                            p_avcbs_buf->get_size(),
                                            b_is_video) != NLFS_RETCODE_OK)
                        return NLFS_RETCODE_ERR_WRITE;
                }
            }
        }
    }
    else
    {

        if (sfd[1] == -1) 
            sfd[1] = open((stream_name+".aac").c_str(),O_WRONLY|O_NONBLOCK);

        if(sfd[1] != -1)
        {
            if(write_to_substr_fifo(p_outbuf, outbuf_bytes, b_is_video) != NLFS_RETCODE_OK)
                return NLFS_RETCODE_ERR_WRITE;
        }
    }
    
    return NLFS_RETCODE_OK;
}