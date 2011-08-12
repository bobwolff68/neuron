#include "v4fifoout.h"

using namespace std;

v4_fifoout_t::v4_fifoout_t(const char* _fifo_out, v4_rtenc_t* _p_rtenc):
fifo_out(_fifo_out),p_rtenc(_p_rtenc),b_sps_sent(false),b_pps_sent(false)
{
    p_avcbs_buf = new v4_avcbsbuf_t();
    p_sps_buf = new v4_avcbsbuf_t();
    p_pps_buf = new v4_avcbsbuf_t();
}


v4_fifoout_t::~v4_fifoout_t()
{
    delete p_avcbs_buf;
    delete p_sps_buf;
    delete p_pps_buf;
}

int v4_fifoout_t::workerBee(void)
{
    int retcode;
   
    LOG_OUT("checkpoint: test if output fifo exists");

    if(access(fifo_out,F_OK)==-1)
    {
        if(mkfifo(fifo_out,S_IWUSR|S_IRUSR)==-1)
        {
            LOG_ERR("mkfifo() error");
            throw V4FIFOOUT_RETCODE_ERR_MKFIFO;
        }
    }

    LOG_OUT("checkpoint: open output fifo");
    fd_fifo_out = open(fifo_out,O_WRONLY|O_NONBLOCK);
    
    while(1)
    {
        if(fd_fifo_out==-1)
            fd_fifo_out = open(fifo_out,O_WRONLY|O_NONBLOCK);
        
        p_rtenc->LockHandle();
        retcode = v4e_get_nal_ex((void*)p_rtenc->Handle(),&p_ms,1);
        p_rtenc->UnlockHandle();
        
        switch(retcode)
        {
            case VSSH_OK:
            {
                int nalu_type = NALU_TYPE(((uint8_t*)(p_ms->data))[0]);
                int nalu_ridc = NALU_RIDC(((uint8_t*)(p_ms->data))[0]);
                
                LOG_OUT("checkpoint: nalu_type="
                        << nalu_type
                        << ", nalu_ridc="
                        << nalu_ridc
                        << ", size="
                        << p_ms->used_size
                        << " (bytes)");

                if (nalu_type == NALU_TYPE_SPS) 
                {
                    if(!b_sps_sent)
                    {
                        p_sps_buf->copy_nalu(p_ms->data, p_ms->used_size);
                    }
                }
                else if (nalu_type == NALU_TYPE_PPS)
                {
                    if(!b_pps_sent)
                    {
                        p_pps_buf->copy_nalu(p_ms->data, p_ms->used_size);
                    }
                }
                else
                {
                    if(fd_fifo_out != -1)
                    {
                        if(nalu_type == NALU_TYPE_IDR || nalu_ridc > 1)
                        {
                            cout << "I frame" << endl;
                            if(!b_sps_sent)
                            {
                                cout << "Sending sps" << endl;
                                if(write(fd_fifo_out,
                                         p_sps_buf->get_membuf_ptr(),
                                         p_sps_buf->get_size())==-1)
                                {
                                    if(close(fd_fifo_out)==-1)
                                    {
                                        LOG_ERR("close() error");
                                        throw V4FIFOOUT_RETCODE_ERR_CLOSEFIFO;
                                    }
                                    fd_fifo_out = -1;
                                    b_sps_sent = false;
                                    b_pps_sent = false;
                                }
                                else
                                    b_sps_sent = true;
                            }
                    
                            if(!b_pps_sent)
                            {
                                cout << "Sending sps" << endl;
                                if(write(fd_fifo_out,
                                         p_pps_buf->get_membuf_ptr(),
                                         p_pps_buf->get_size())==-1)
                                {
                                    if(close(fd_fifo_out)==-1)
                                    {
                                        LOG_ERR("close() error");
                                        throw V4FIFOOUT_RETCODE_ERR_CLOSEFIFO;
                                    }
                                    fd_fifo_out = -1;
                                    b_sps_sent = false;
                                    b_pps_sent = false;
                                }
                                else
                                    b_pps_sent = true;                        
                            }
                        }
                    
                        if(b_sps_sent && b_pps_sent)
                        {
                            p_avcbs_buf->copy_nalu(p_ms->data,p_ms->used_size);
                            if(write(fd_fifo_out,
                                     p_avcbs_buf->get_membuf_ptr(),
                                     p_avcbs_buf->get_size())==-1)
                            {
                                if(close(fd_fifo_out)==-1)
                                {
                                    LOG_ERR("close() error");
                                    throw V4FIFOOUT_RETCODE_ERR_CLOSEFIFO;
                                }
                                fd_fifo_out = -1;
                                b_sps_sent = false;
                                b_pps_sent = false;
                            }
                        }
                    }
                }
                
                v4_free_media_sample(p_ms);
                break;
            }

            case VSSH_WARN_EOS:
            {
                LOG_OUT("checkpoint: end of stream");                
                if(fd_fifo_out!=-1 && close(fd_fifo_out)==-1)
                {
                    LOG_ERR("close() error");
                    throw V4FIFOOUT_RETCODE_ERR_CLOSEFIFO;
                }
                return 0;
            }

            default:
            {
                usleep(15000);
                break;
            }
        }
    }

    return 0;
}